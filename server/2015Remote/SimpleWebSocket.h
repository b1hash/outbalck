#pragma once
// SimpleWebSocket - A lightweight WebSocket server implementation
// Compatible with httplib and doesn't require Windows 10

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <wincrypt.h>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <thread>
#include <functional>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <fstream>
#include <filesystem>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "advapi32.lib")

namespace ws {

namespace fs = std::filesystem;

// Static shutdown flag - survives Server destruction, safe for detached threads
static std::atomic<bool> s_serverShuttingDown{false};

// HTTP Response structure for enhanced HTTP handling
struct HttpResponse {
    int status = 200;
    std::string contentType = "text/html; charset=utf-8";
    std::map<std::string, std::string> headers;
    std::string body;
    std::string filePath;  // If set, stream file instead of body

    HttpResponse() = default;
    HttpResponse(int s) : status(s) {}
    HttpResponse(const std::string& content) : body(content) {}

    static HttpResponse NotFound() { return HttpResponse(404); }
    static HttpResponse Forbidden() { return HttpResponse(403); }
    static HttpResponse OK(const std::string& content, const std::string& type = "text/html; charset=utf-8") {
        HttpResponse r;
        r.body = content;
        r.contentType = type;
        return r;
    }
    static HttpResponse File(const std::string& path, const std::string& filename = "") {
        HttpResponse r;
        r.filePath = path;
        r.contentType = "application/octet-stream";
        if (!filename.empty()) {
            r.headers["Content-Disposition"] = "attachment; filename=\"" + filename + "\"";
        }
        return r;
    }
};

// WebSocket opcodes
enum Opcode {
    CONTINUATION = 0x0,
    TEXT = 0x1,
    BINARY = 0x2,
    CLOSE = 0x8,
    PING = 0x9,
    PONG = 0xA
};

// WebSocket frame
struct Frame {
    bool fin;
    Opcode opcode;
    std::vector<uint8_t> payload;
};

// WebSocket connection
class Connection {
public:
    Connection(SOCKET sock, const std::string& ip)
        : m_socket(sock), m_clientIP(ip), m_closed(false) {}

    ~Connection() { close(); }

    bool send(const std::string& text) {
        return sendFrame(TEXT, (const uint8_t*)text.data(), text.size());
    }

    bool sendBinary(const uint8_t* data, size_t len) {
        return sendFrame(BINARY, data, len);
    }

    bool sendPing() {
        return sendFrame(PING, nullptr, 0);
    }

    bool sendFrame(Opcode opcode, const uint8_t* data, size_t len) {
        if (m_closed) return false;

        std::vector<uint8_t> frame;

        // FIN + opcode
        frame.push_back(0x80 | opcode);

        // Payload length (server doesn't mask)
        if (len < 126) {
            frame.push_back((uint8_t)len);
        } else if (len < 65536) {
            frame.push_back(126);
            frame.push_back((len >> 8) & 0xFF);
            frame.push_back(len & 0xFF);
        } else {
            frame.push_back(127);
            for (int i = 7; i >= 0; i--) {
                frame.push_back((len >> (i * 8)) & 0xFF);
            }
        }

        // Payload
        frame.insert(frame.end(), data, data + len);

        std::lock_guard<std::mutex> lock(m_sendMutex);
        int sent = ::send(m_socket, (const char*)frame.data(), (int)frame.size(), 0);
        return sent == (int)frame.size();
    }

    bool readFrame(Frame& frame) {
        if (m_closed) return false;

        uint8_t header[2];
        if (recv(m_socket, (char*)header, 2, MSG_WAITALL) != 2) {
            return false;
        }

        frame.fin = (header[0] & 0x80) != 0;
        frame.opcode = (Opcode)(header[0] & 0x0F);

        bool masked = (header[1] & 0x80) != 0;
        uint64_t payloadLen = header[1] & 0x7F;

        if (payloadLen == 126) {
            uint8_t ext[2];
            if (recv(m_socket, (char*)ext, 2, MSG_WAITALL) != 2) return false;
            payloadLen = (ext[0] << 8) | ext[1];
        } else if (payloadLen == 127) {
            uint8_t ext[8];
            if (recv(m_socket, (char*)ext, 8, MSG_WAITALL) != 8) return false;
            payloadLen = 0;
            for (int i = 0; i < 8; i++) {
                payloadLen = (payloadLen << 8) | ext[i];
            }
        }

        uint8_t mask[4] = {0};
        if (masked) {
            if (recv(m_socket, (char*)mask, 4, MSG_WAITALL) != 4) return false;
        }

        frame.payload.resize((size_t)payloadLen);
        if (payloadLen > 0) {
            if (recv(m_socket, (char*)frame.payload.data(), (int)payloadLen, MSG_WAITALL) != (int)payloadLen) {
                return false;
            }

            // Unmask
            if (masked) {
                for (size_t i = 0; i < payloadLen; i++) {
                    frame.payload[i] ^= mask[i % 4];
                }
            }
        }

        return true;
    }

    void close() {
        if (!m_closed.exchange(true)) {
            closesocket(m_socket);
        }
    }

    bool isClosed() const { return m_closed; }
    const std::string& clientIP() const { return m_clientIP; }
    SOCKET socket() const { return m_socket; }

private:
    SOCKET m_socket;
    std::string m_clientIP;
    std::atomic<bool> m_closed;
    std::mutex m_sendMutex;
};

// WebSocket server
class Server {
public:
    using MessageHandler = std::function<void(std::shared_ptr<Connection>, const std::string&)>;
    using BinaryHandler = std::function<void(std::shared_ptr<Connection>, const uint8_t*, size_t)>;
    using ConnectHandler = std::function<void(std::shared_ptr<Connection>)>;
    using DisconnectHandler = std::function<void(std::shared_ptr<Connection>)>;

    Server() : m_listenSocket(INVALID_SOCKET), m_running(false), m_activeThreads(0) {}
    ~Server() { stop(); }

    void onMessage(MessageHandler handler) { m_onMessage = handler; }
    void onBinary(BinaryHandler handler) { m_onBinary = handler; }
    void onConnect(ConnectHandler handler) { m_onConnect = handler; }
    void onDisconnect(DisconnectHandler handler) { m_onDisconnect = handler; }

    bool start(int port, const std::string& path = "/ws") {
        m_path = path;

        // Initialize Winsock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return false;
        }

        // Create socket
        m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_listenSocket == INVALID_SOCKET) {
            WSACleanup();
            return false;
        }

        // Allow reuse
        int opt = 1;
        setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

        // Bind
        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons((u_short)port);

        if (bind(m_listenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            closesocket(m_listenSocket);
            WSACleanup();
            return false;
        }

        // Listen
        if (listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR) {
            closesocket(m_listenSocket);
            WSACleanup();
            return false;
        }

        m_running = true;
        m_acceptThread = std::thread(&Server::acceptLoop, this);
        return true;
    }

    void stop() {
        if (!m_running) return;
        m_running = false;
        s_serverShuttingDown = true;  // Static flag survives Server destruction

        // Close listen socket first to stop accepting new connections
        closesocket(m_listenSocket);

        if (m_acceptThread.joinable()) {
            m_acceptThread.join();
        }

        // Close all connections (this will cause handleClient loops to exit)
        {
            std::lock_guard<std::mutex> lock(m_connectionsMutex);
            for (auto& conn : m_connections) {
                conn->close();
            }
        }

        // Wait for all handleClient threads to exit (max 5 seconds)
        {
            std::unique_lock<std::mutex> lock(m_threadCountMutex);
            bool allExited = m_threadCountCV.wait_for(lock, std::chrono::seconds(5), [this]() {
                return m_activeThreads == 0;
            });
            if (!allExited) {
                // Threads didn't exit in time - they will exit on next recv() timeout
                // Log warning but continue shutdown (don't block forever)
                OutputDebugStringA("[WebSocket] Warning: Some threads still active after timeout\n");
            }
        }

        // Now safe to clear callbacks after threads have exited
        {
            std::lock_guard<std::mutex> lock(m_connectionsMutex);
            m_connections.clear();
        }
        m_httpHandler = nullptr;
        m_onConnect = nullptr;
        m_onDisconnect = nullptr;
        m_onMessage = nullptr;
        m_onBinary = nullptr;

        WSACleanup();
    }

    void broadcast(const std::string& text) {
        std::lock_guard<std::mutex> lock(m_connectionsMutex);
        for (auto& conn : m_connections) {
            conn->send(text);
        }
    }

    void broadcastBinary(const uint8_t* data, size_t len) {
        std::lock_guard<std::mutex> lock(m_connectionsMutex);
        for (auto& conn : m_connections) {
            conn->sendBinary(data, len);
        }
    }

    // Send WebSocket ping to all connections (for heartbeat)
    void pingAll() {
        std::lock_guard<std::mutex> lock(m_connectionsMutex);
        for (auto& conn : m_connections) {
            conn->sendPing();
        }
    }

    bool isRunning() const { return m_running; }

private:
    void acceptLoop() {
        while (m_running) {
            sockaddr_in clientAddr;
            int addrLen = sizeof(clientAddr);

            SOCKET clientSocket = accept(m_listenSocket, (sockaddr*)&clientAddr, &addrLen);
            if (clientSocket == INVALID_SOCKET) {
                continue;
            }

            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, INET_ADDRSTRLEN);

            std::thread(&Server::handleClient, this, clientSocket, std::string(ipStr)).detach();
        }
    }

    // HTTP handler for non-WebSocket requests
    using HttpHandler = std::function<HttpResponse(const std::string& path)>;
    HttpHandler m_httpHandler;

public:
    void onHttp(HttpHandler handler) { m_httpHandler = handler; }

private:
    // Parse Proxy Protocol v2 header and extract real client IP
    bool parseProxyProtocolV2(SOCKET sock, std::string& clientIP) {
        // PP2 signature (12 bytes)
        static const uint8_t PP2_SIG[] = {
            0x0D, 0x0A, 0x0D, 0x0A, 0x00, 0x0D, 0x0A,
            0x51, 0x55, 0x49, 0x54, 0x0A
        };

        // Peek first 16 bytes (signature + ver/cmd + fam + len)
        uint8_t header[16];
        int n = recv(sock, (char*)header, 16, MSG_PEEK);
        if (n < 16) return false;

        // Check signature
        if (memcmp(header, PP2_SIG, 12) != 0) return false;

        // Actually consume the 16 bytes
        recv(sock, (char*)header, 16, MSG_WAITALL);

        uint8_t verCmd = header[12];
        uint8_t fam = header[13];
        uint16_t addrLen = (header[14] << 8) | header[15];

        // Version must be 2, command 0 (LOCAL) or 1 (PROXY)
        if ((verCmd & 0xF0) != 0x20) return false;

        // Read address data
        if (addrLen > 0 && addrLen <= 512) {
            std::vector<uint8_t> addrData(addrLen);
            if (recv(sock, (char*)addrData.data(), addrLen, MSG_WAITALL) != addrLen) {
                return false;
            }

            // Extract client IP if PROXY command with IPv4
            if ((verCmd & 0x0F) == 0x01 && (fam & 0xF0) == 0x10) {
                // IPv4: src_addr(4) + dst_addr(4) + src_port(2) + dst_port(2)
                if (addrLen >= 12) {
                    char ip[INET_ADDRSTRLEN];
                    struct in_addr addr;
                    memcpy(&addr, addrData.data(), 4);
                    inet_ntop(AF_INET, &addr, ip, sizeof(ip));
                    clientIP = ip;
                }
            }
            // IPv6: src_addr(16) + dst_addr(16) + src_port(2) + dst_port(2)
            else if ((verCmd & 0x0F) == 0x01 && (fam & 0xF0) == 0x20) {
                if (addrLen >= 36) {
                    char ip[INET6_ADDRSTRLEN];
                    struct in6_addr addr;
                    memcpy(&addr, addrData.data(), 16);
                    inet_ntop(AF_INET6, &addr, ip, sizeof(ip));
                    clientIP = ip;
                }
            }
        }
        return true;
    }

    void handleClient(SOCKET sock, std::string clientIP) {
        // Check if already shutting down before starting
        if (s_serverShuttingDown) {
            closesocket(sock);
            return;
        }

        // Increment active thread count
        {
            std::lock_guard<std::mutex> lock(m_threadCountMutex);
            m_activeThreads++;
        }

        // RAII guard to decrement thread count on exit
        // Note: Must check static flag to avoid accessing destroyed Server
        struct ThreadGuard {
            Server* server;
            ~ThreadGuard() {
                // Only access server members if not shutting down
                if (!s_serverShuttingDown) {
                    std::lock_guard<std::mutex> lock(server->m_threadCountMutex);
                    server->m_activeThreads--;
                    server->m_threadCountCV.notify_all();
                }
            }
        } guard{this};

        // Check for Proxy Protocol v2 header
        parseProxyProtocolV2(sock, clientIP);

        // Read HTTP request
        std::string request;
        char buf[4096];
        int n;

        while ((n = recv(sock, buf, sizeof(buf) - 1, 0)) > 0) {
            buf[n] = 0;
            request += buf;
            if (request.find("\r\n\r\n") != std::string::npos) break;
        }

        // Check if WebSocket upgrade
        if (request.find("Upgrade: websocket") == std::string::npos) {
            // Regular HTTP request - extract path
            std::string path = "/";
            size_t getPos = request.find("GET ");
            if (getPos != std::string::npos) {
                size_t pathStart = getPos + 4;
                size_t pathEnd = request.find(" ", pathStart);
                if (pathEnd != std::string::npos) {
                    path = request.substr(pathStart, pathEnd - pathStart);
                }
            }

            // Check if server is shutting down (use static flag - safe after Server destruction)
            if (s_serverShuttingDown) {
                closesocket(sock);
                return;
            }

            HttpResponse resp(404);
            if (m_running && m_httpHandler) {
                resp = m_httpHandler(path);
            }

            // Build and send HTTP response
            sendHttpResponse(sock, resp);
            closesocket(sock);
            return;
        }

        // Extract Sec-WebSocket-Key
        std::string key;
        size_t keyPos = request.find("Sec-WebSocket-Key:");
        if (keyPos != std::string::npos) {
            keyPos += 18;
            while (keyPos < request.size() && request[keyPos] == ' ') keyPos++;
            size_t keyEnd = request.find("\r\n", keyPos);
            if (keyEnd != std::string::npos) {
                key = request.substr(keyPos, keyEnd - keyPos);
            }
        }

        if (key.empty()) {
            closesocket(sock);
            return;
        }

        // Generate accept key
        std::string acceptKey = generateAcceptKey(key);

        // Send handshake response
        std::string response =
            "HTTP/1.1 101 Switching Protocols\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: " + acceptKey + "\r\n"
            "\r\n";

        ::send(sock, response.c_str(), (int)response.size(), 0);

        // Create connection
        auto conn = std::make_shared<Connection>(sock, clientIP);

        // Check if server is shutting down (use static flag - safe after Server destruction)
        if (s_serverShuttingDown) {
            closesocket(sock);
            return;
        }

        {
            std::lock_guard<std::mutex> lock(m_connectionsMutex);
            m_connections.push_back(conn);
        }

        // Copy callback to local variable to avoid race condition
        auto onConnect = m_onConnect;
        if (!s_serverShuttingDown && m_running && onConnect) {
            onConnect(conn);
        }

        // Message loop - use static flag for outer check, member for inner
        Frame frame;
        while (!s_serverShuttingDown && m_running && !conn->isClosed()) {
            if (!conn->readFrame(frame)) {
                break;
            }

            // Check shutdown status again before processing
            if (s_serverShuttingDown) break;

            switch (frame.opcode) {
            case TEXT:
                {
                    auto onMessage = m_onMessage;
                    if (!s_serverShuttingDown && m_running && onMessage) {
                        std::string msg(frame.payload.begin(), frame.payload.end());
                        onMessage(conn, msg);
                    }
                }
                break;
            case BINARY:
                {
                    auto onBinary = m_onBinary;
                    if (!s_serverShuttingDown && m_running && onBinary) {
                        onBinary(conn, frame.payload.data(), frame.payload.size());
                    }
                }
                break;
            case PING:
                if (!s_serverShuttingDown) {
                    conn->sendFrame(PONG, frame.payload.data(), frame.payload.size());
                }
                break;
            case CLOSE:
                conn->close();
                break;
            default:
                break;
            }
        }

        // Cleanup
        conn->close();

        // Only call onDisconnect if server is still running (check static flag first)
        if (!s_serverShuttingDown) {
            auto onDisconnect = m_onDisconnect;
            if (m_running && onDisconnect) {
                onDisconnect(conn);
            }

            // Remove from connections list
            {
                std::lock_guard<std::mutex> lock(m_connectionsMutex);
                m_connections.erase(
                    std::remove_if(m_connections.begin(), m_connections.end(),
                        [&conn](const std::shared_ptr<Connection>& c) { return c.get() == conn.get(); }),
                    m_connections.end());
            }
        }
    }

    std::string generateAcceptKey(const std::string& key) {
        // WebSocket magic GUID
        std::string concat = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

        // SHA-1 hash
        HCRYPTPROV hProv = 0;
        HCRYPTHASH hHash = 0;
        BYTE hash[20];
        DWORD hashLen = 20;

        if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
            return "";
        }

        if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
            CryptReleaseContext(hProv, 0);
            return "";
        }

        CryptHashData(hHash, (BYTE*)concat.c_str(), (DWORD)concat.size(), 0);
        CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0);

        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);

        // Base64 encode
        return base64Encode(hash, 20);
    }

    std::string base64Encode(const uint8_t* data, size_t len) {
        static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;

        int val = 0, valb = -6;
        for (size_t i = 0; i < len; i++) {
            val = (val << 8) + data[i];
            valb += 8;
            while (valb >= 0) {
                result.push_back(chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) {
            result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
        }
        while (result.size() % 4) {
            result.push_back('=');
        }
        return result;
    }

    void sendHttpResponse(SOCKET sock, const HttpResponse& resp) {
        std::string statusText;
        switch (resp.status) {
            case 200: statusText = "OK"; break;
            case 403: statusText = "Forbidden"; break;
            case 404: statusText = "Not Found"; break;
            case 500: statusText = "Internal Server Error"; break;
            default: statusText = "Unknown"; break;
        }

        // File streaming response
        if (!resp.filePath.empty()) {
            std::error_code ec;
            if (!fs::exists(resp.filePath, ec) || !fs::is_regular_file(resp.filePath, ec)) {
                std::string errResp = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
                ::send(sock, errResp.c_str(), (int)errResp.size(), 0);
                return;
            }

            uintmax_t fileSize = fs::file_size(resp.filePath, ec);
            if (ec) {
                std::string errResp = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
                ::send(sock, errResp.c_str(), (int)errResp.size(), 0);
                return;
            }

            // Build headers
            std::string headers = "HTTP/1.1 200 OK\r\n";
            headers += "Content-Type: " + resp.contentType + "\r\n";
            headers += "Content-Length: " + std::to_string(fileSize) + "\r\n";
            for (const auto& [key, val] : resp.headers) {
                headers += key + ": " + val + "\r\n";
            }
            headers += "Connection: close\r\n\r\n";

            ::send(sock, headers.c_str(), (int)headers.size(), 0);

            // Stream file content
            std::ifstream file(resp.filePath, std::ios::binary);
            if (file) {
                char buffer[65536];
                while (file) {
                    file.read(buffer, sizeof(buffer));
                    std::streamsize n = file.gcount();
                    if (n > 0) {
                        int sent = 0;
                        while (sent < n) {
                            int r = ::send(sock, buffer + sent, (int)(n - sent), 0);
                            if (r <= 0) break;
                            sent += r;
                        }
                        if (sent < n) break;  // Send failed
                    }
                }
            }
            return;
        }

        // Regular body response
        std::string response = "HTTP/1.1 " + std::to_string(resp.status) + " " + statusText + "\r\n";
        response += "Content-Type: " + resp.contentType + "\r\n";
        response += "Content-Length: " + std::to_string(resp.body.size()) + "\r\n";
        for (const auto& [key, val] : resp.headers) {
            response += key + ": " + val + "\r\n";
        }
        response += "Connection: close\r\n\r\n";
        response += resp.body;

        ::send(sock, response.c_str(), (int)response.size(), 0);
    }

private:
    SOCKET m_listenSocket;
    std::string m_path;
    std::atomic<bool> m_running;
    std::thread m_acceptThread;

    std::vector<std::shared_ptr<Connection>> m_connections;
    std::mutex m_connectionsMutex;

    // Thread counting for safe shutdown
    std::atomic<int> m_activeThreads;
    std::mutex m_threadCountMutex;
    std::condition_variable m_threadCountCV;

    MessageHandler m_onMessage;
    BinaryHandler m_onBinary;
    ConnectHandler m_onConnect;
    DisconnectHandler m_onDisconnect;
};

} // namespace ws
