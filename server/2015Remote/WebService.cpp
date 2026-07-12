#include "stdafx.h"
#include "2015Remote.h"
#include "WebService.h"
#include "WebServiceAuth.h"
#include "2015RemoteDlg.h"
#include "Server.h"  // For CONTEXT_OBJECT
#include "jsoncpp/json.h"
#include "WebPage.h"
#include "SimpleWebSocket.h"
#include "common/commands.h"
#include <filesystem>

// Algorithm constants (same as ScreenSpyDlg.cpp)
#define ALGORITHM_H264 2

#pragma comment(lib, "ws2_32.lib")

// Challenge-response nonce storage (prevents replay attacks)
static std::map<void*, std::string> s_ClientNonces;
static std::mutex s_NonceMutex;
static std::atomic<bool> s_bShuttingDown{false};  // Prevents access during static destruction

// Generate random nonce (32 hex chars) - thread-safe
static std::string GenerateNonce() {
    if (s_bShuttingDown) return "";
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    char buf[33];
    {
        std::lock_guard<std::mutex> lock(s_NonceMutex);
        sprintf_s(buf, "%016llx%016llx", dis(gen), dis(gen));
    }
    return buf;
}

// Store nonce for client
static void StoreNonce(void* ws_ptr, const std::string& nonce) {
    if (s_bShuttingDown) return;
    std::lock_guard<std::mutex> lock(s_NonceMutex);
    s_ClientNonces[ws_ptr] = nonce;
}

// Get and clear nonce for client (one-time use)
static std::string ConsumeNonce(void* ws_ptr) {
    if (s_bShuttingDown) return "";
    std::lock_guard<std::mutex> lock(s_NonceMutex);
    auto it = s_ClientNonces.find(ws_ptr);
    if (it == s_ClientNonces.end()) return "";
    std::string nonce = it->second;
    s_ClientNonces.erase(it);
    return nonce;
}

// Clear nonce when client disconnects
static void ClearNonce(void* ws_ptr) {
    if (s_bShuttingDown) return;
    std::lock_guard<std::mutex> lock(s_NonceMutex);
    s_ClientNonces.erase(ws_ptr);
}

// Helper: Convert ANSI (GBK) string to UTF-8
static std::string AnsiToUtf8(const CString& str) {
    if (str.IsEmpty()) return "";
#ifdef _UNICODE
    // Unicode build: CString is already UTF-16, convert to UTF-8
    int len = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    if (len <= 0) return "";
    std::string result(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, str, -1, &result[0], len, NULL, NULL);
    return result;
#else
    // MBCS build: CString is ANSI (GBK), need to convert to UTF-16 first, then to UTF-8
    int wlen = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    if (wlen <= 0) return "";
    std::wstring wstr(wlen - 1, L'\0');
    MultiByteToWideChar(CP_ACP, 0, str, -1, &wstr[0], wlen);

    int u8len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (u8len <= 0) return "";
    std::string result(u8len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], u8len, NULL, NULL);
    return result;
#endif
}

//////////////////////////////////////////////////////////////////////////
// CWebService Implementation
//////////////////////////////////////////////////////////////////////////

CWebService& CWebService::Instance() {
    static CWebService instance;
    return instance;
}

CWebService::CWebService()
    : m_bRunning(false)
    , m_bStopping(false)
    , m_pServer(nullptr)
    , m_pParentDlg(nullptr)
    , m_nMaxClientsPerDevice(10)
    , m_nTokenExpireSeconds(86400)  // 24 hours
    , m_bHideWebSessions(true)      // Hide web-triggered dialogs by default
{
    // Secret key will be initialized in Start() via WSAuth::Init()
    // Admin user will be configured via SetAdminPassword()

    // Initialize payloads directory
    char exe_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    std::filesystem::path exeDir = std::filesystem::path(exe_path).parent_path();
    m_PayloadsDir = (exeDir / "Payloads").string();
    std::error_code ec;
    std::filesystem::create_directories(m_PayloadsDir, ec);
}

void CWebService::SetAdminPassword(const std::string& password) {
    m_Users.clear();

    WebUser admin;
    admin.username = "admin";
    admin.salt = "";  // Not used with challenge-response auth
    admin.password_hash = WSAuth::ComputeSHA256(password);  // Simple SHA256
    admin.role = "admin";
    m_Users.push_back(admin);

    Mprintf("[WebService] Admin password configured\n");
}

CWebService::~CWebService() {
    Stop();
}

bool CWebService::Start(int port) {
    if (m_bRunning) return true;

    // Initialize authorization and get secret key
    // Pass raw authorization string, verified internally by private library
    WSAuthContext authCtx;
    std::string authStr = THIS_CFG.GetStr("settings", "Authorization", "");
    if (!WSAuth::Init(authCtx, authStr)) {
        Mprintf("[WebService] Authorization check failed, service disabled\n");
        return false;
    }

    m_SecretKey = authCtx.secretKey;
    m_nTokenExpireSeconds = authCtx.tokenExpireSec;
    m_nMaxClientsPerDevice = authCtx.maxClientsPerDevice;

    m_bStopping = false;
    m_ServerThread = std::thread(&CWebService::ServerThread, this, port);

    // Wait for server to start
    for (int i = 0; i < 50 && !m_bRunning; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Start heartbeat thread
    if (m_bRunning) {
        m_HeartbeatThread = std::thread(&CWebService::HeartbeatThread, this);
    }

    return m_bRunning;
}

void CWebService::Stop() {
    if (!m_bRunning) return;

    // Set flags FIRST to prevent new operations from starting
    m_bRunning = false;
    m_bStopping = true;
    s_bShuttingDown = true;  // Prevent access to static variables

    // Stop heartbeat thread first
    if (m_HeartbeatThread.joinable()) {
        m_HeartbeatThread.join();
    }

    if (m_ServerThread.joinable()) {
        m_ServerThread.join();
    }

    // Clear all clients after server stopped
    {
        std::lock_guard<std::mutex> lock(m_ClientsMutex);
        m_Clients.clear();
    }

    // Clear screen contexts to prevent dangling pointers
    {
        std::lock_guard<std::mutex> lock(m_ScreenContextsMutex);
        m_ScreenContexts.clear();
    }

    // Clear device cache
    {
        std::lock_guard<std::mutex> lock(m_DeviceCacheMutex);
        m_DeviceCache.clear();
    }

    m_pServer = nullptr;
    m_pParentDlg = nullptr;  // Clear to prevent access to destroyed dialog
}

bool CWebService::IsRunning() const {
    return m_bRunning;
}

void CWebService::ServerThread(int port) {
    ws::Server wsServer;
    m_pServer = &wsServer;

    // Serve static HTML page and file downloads
    static std::string cachedHtml = GetWebPageHTML();
    std::string payloadsDir = m_PayloadsDir;  // Capture for lambda

    wsServer.onHttp([payloadsDir](const std::string& path) -> ws::HttpResponse {
        if (path == "/" || path == "/index.html") {
            return ws::HttpResponse::OK(cachedHtml);
        } else if (path == "/health") {
            return ws::HttpResponse::OK("{\"status\":\"ok\"}", "application/json");
        } else if (path == "/manifest.json") {
            // PWA manifest for iOS standalone app support
            static const char* manifest = R"({
  "name": "SimpleRemoter",
  "short_name": "Remoter",
  "start_url": "/",
  "display": "standalone",
  "orientation": "any",
  "background_color": "#1a1a2e",
  "theme_color": "#1a1a2e"
})";
            return ws::HttpResponse::OK(manifest, "application/manifest+json");
        } else if (path.rfind("/payloads/", 0) == 0) {
            // File download: /payloads/filename
            std::string filename = path.substr(10);  // Remove "/payloads/"

            // Security check: no path traversal or absolute paths
            if (filename.empty() ||
                filename.find("..") != std::string::npos ||  // Path traversal
                filename[0] == '/' || filename[0] == '\\' ||  // Absolute path (Unix/Windows)
                (filename.length() > 1 && filename[1] == ':'))  // Windows drive letter (C:)
            {
                return ws::HttpResponse::Forbidden();
            }

            std::filesystem::path filepath = std::filesystem::path(payloadsDir) / filename;
            std::error_code ec;
            if (!std::filesystem::exists(filepath, ec) || !std::filesystem::is_regular_file(filepath, ec)) {
                return ws::HttpResponse::NotFound();
            }

            return ws::HttpResponse::File(filepath.string(), filename);
        }
        return ws::HttpResponse::NotFound();
    });

    // WebSocket connect
    wsServer.onConnect([this](std::shared_ptr<ws::Connection> conn) {
        // Skip if server is stopping
        if (m_bStopping) return;

        void* ws_ptr = conn.get();
        RegisterClient(ws_ptr, conn->clientIP());

        // Generate and send challenge nonce
        std::string nonce = GenerateNonce();
        if (nonce.empty()) return;  // Shutting down
        StoreNonce(ws_ptr, nonce);

        Json::Value challenge;
        challenge["cmd"] = "challenge";
        challenge["nonce"] = nonce;
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";
        SendText(ws_ptr, Json::writeString(builder, challenge));
    });

    // WebSocket disconnect
    wsServer.onDisconnect([this](std::shared_ptr<ws::Connection> conn) {
        // Skip cleanup if server is stopping (destructor will clean up)
        if (m_bStopping) return;
        void* ws_ptr = conn.get();
        ClearNonce(ws_ptr);
        UnregisterClient(ws_ptr);
    });

    // WebSocket message
    wsServer.onMessage([this](std::shared_ptr<ws::Connection> conn, const std::string& msg) {
        // Skip if server is stopping
        if (m_bStopping) return;

        void* ws_ptr = conn.get();

        // Update last activity time for heartbeat
        {
            std::lock_guard<std::mutex> lock(m_ClientsMutex);
            auto it = m_Clients.find(ws_ptr);
            if (it != m_Clients.end()) {
                it->second.last_activity = (uint64_t)time(nullptr);
            }
        }

        // Parse JSON signaling
        Json::Value root;
        Json::Reader reader;
        if (reader.parse(msg, root)) {
            std::string cmd = root.get("cmd", "").asString();
            std::string token = root.get("token", "").asString();

            if (cmd == "login") {
                HandleLogin(ws_ptr, msg, conn->clientIP());
            } else if (cmd == "get_devices") {
                HandleGetDevices(ws_ptr, token);
            } else if (cmd == "connect") {
                // Support both string and number ID formats
                std::string id_str = root.get("id", "").asString();
                uint64_t device_id = id_str.empty() ? 0 : strtoull(id_str.c_str(), nullptr, 10);
                HandleConnect(ws_ptr, token, device_id);
            } else if (cmd == "disconnect") {
                HandleDisconnect(ws_ptr, token);
            } else if (cmd == "ping") {
                HandlePing(ws_ptr, token);
            } else if (cmd == "mouse") {
                HandleMouse(ws_ptr, msg);
            } else if (cmd == "key") {
                HandleKey(ws_ptr, msg);
            } else if (cmd == "rdp_reset") {
                HandleRdpReset(ws_ptr, token);
            }
        }
    });

    // Start listening
    if (wsServer.start(port)) {
        m_bRunning = true;
        // Wait until stop is requested
        while (!m_bStopping && wsServer.isRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        wsServer.stop();
    }
    m_bRunning = false;
}

void CWebService::HeartbeatThread() {
    Mprintf("[WebService] Heartbeat thread started (interval=%ds, timeout=%ds)\n",
            HEARTBEAT_INTERVAL_SEC, HEARTBEAT_TIMEOUT_SEC);

    while (!m_bStopping) {
        // Sleep in small increments to respond quickly to stop
        for (int i = 0; i < HEARTBEAT_INTERVAL_SEC * 10 && !m_bStopping; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (m_bStopping) break;

        // Send ping to all WebSocket connections
        // Copy pointer first to avoid race with Stop()
        void* pServer = m_pServer;
        if (pServer && !m_bStopping) {
            auto* wsServer = static_cast<ws::Server*>(pServer);
            if (wsServer->isRunning()) {
                wsServer->pingAll();
            }
        }

        // Check for timed out clients
        uint64_t now = (uint64_t)time(nullptr);
        std::vector<void*> timedOutClients;

        {
            std::lock_guard<std::mutex> lock(m_ClientsMutex);
            for (const auto& [ws_ptr, client] : m_Clients) {
                // Only check clients that are watching a device (streaming video)
                if (client.watch_device_id > 0 && client.last_activity > 0) {
                    uint64_t elapsed = now - client.last_activity;
                    if (elapsed > HEARTBEAT_TIMEOUT_SEC) {
                        timedOutClients.push_back(ws_ptr);
                        Mprintf("[WebService] Client %s timed out (no activity for %llu seconds)\n",
                                client.client_ip.c_str(), elapsed);
                    }
                }
            }
        }

        // Disconnect timed out clients (outside lock to avoid deadlock)
        for (void* ws_ptr : timedOutClients) {
            UnregisterClient(ws_ptr);
        }
    }

    Mprintf("[WebService] Heartbeat thread stopped\n");
}

//////////////////////////////////////////////////////////////////////////
// Signaling Handlers
//////////////////////////////////////////////////////////////////////////

void CWebService::HandleLogin(void* ws_ptr, const std::string& msg, const std::string& client_ip) {
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(msg, root)) {
        SendText(ws_ptr, BuildJsonResponse("login_result", false, "Invalid JSON"));
        return;
    }

    std::string username = root.get("username", "").asString();
    std::string response = root.get("response", "").asString();
    std::string clientNonce = root.get("nonce", "").asString();

    // Verify nonce matches (prevents replay)
    std::string storedNonce = ConsumeNonce(ws_ptr);
    if (storedNonce.empty() || storedNonce != clientNonce) {
        SendText(ws_ptr, BuildJsonResponse("login_result", false, "Invalid or expired challenge"));
        return;
    }

    // Check rate limit
    if (!CheckRateLimit(client_ip)) {
        SendText(ws_ptr, BuildJsonResponse("login_result", false, "Too many failed attempts. Try again later."));
        return;
    }

    // Check if password is configured
    if (m_Users.empty()) {
        SendText(ws_ptr, BuildJsonResponse("login_result", false, "Server password not configured"));
        return;
    }

    // Find user
    WebUser* user = nullptr;
    for (auto& u : m_Users) {
        if (u.username == username) {
            user = &u;
            break;
        }
    }

    if (!user) {
        RecordFailedLogin(client_ip);
        SendText(ws_ptr, BuildJsonResponse("login_result", false, "Invalid credentials"));
        return;
    }

    // Verify challenge response: response = SHA256(passwordHash + nonce)
    std::string expectedResponse = WSAuth::ComputeSHA256(user->password_hash + clientNonce);
    if (response != expectedResponse) {
        RecordFailedLogin(client_ip);
        SendText(ws_ptr, BuildJsonResponse("login_result", false, "Invalid credentials"));
        return;
    }

    // Success - generate token
    RecordSuccessLogin(client_ip);
    std::string token = GenerateToken(username, user->role);

    // Update client state
    {
        std::lock_guard<std::mutex> lock(m_ClientsMutex);
        auto it = m_Clients.find(ws_ptr);
        if (it != m_Clients.end()) {
            it->second.token = token;
            it->second.username = username;
            it->second.role = user->role;
        }
    }

    // Build response
    Json::Value res;
    res["cmd"] = "login_result";
    res["ok"] = true;
    res["token"] = token;
    res["role"] = user->role;
    res["expires"] = m_nTokenExpireSeconds;

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    SendText(ws_ptr, Json::writeString(builder, res));
}

void CWebService::HandleGetDevices(void* ws_ptr, const std::string& token) {
    std::string username, role;
    if (!ValidateToken(token, username, role)) {
        SendText(ws_ptr, BuildJsonResponse("device_list", false, "Invalid token"));
        return;
    }

    SendText(ws_ptr, BuildDeviceListJson());
}

void CWebService::HandleConnect(void* ws_ptr, const std::string& token, uint64_t device_id) {
    std::string username, role;
    if (!ValidateToken(token, username, role)) {
        SendText(ws_ptr, BuildJsonResponse("connect_result", false, "Invalid token"));
        return;
    }

    // Check device exists and is online
    if (!m_pParentDlg) {
        SendText(ws_ptr, BuildJsonResponse("connect_result", false, "Service not ready"));
        return;
    }

    context* ctx = m_pParentDlg->FindHost(device_id);
    if (!ctx) {
        SendText(ws_ptr, BuildJsonResponse("connect_result", false, "Device not online"));
        return;
    }

    // Check max clients per device
    int current_count = GetWebClientCount(device_id);
    if (current_count >= m_nMaxClientsPerDevice) {
        SendText(ws_ptr, BuildJsonResponse("connect_result", false, "Too many viewers"));
        return;
    }

    // Update client state
    {
        std::lock_guard<std::mutex> lock(m_ClientsMutex);
        auto it = m_Clients.find(ws_ptr);
        if (it != m_Clients.end()) {
            it->second.watch_device_id = device_id;
        }
    }

    // Start remote desktop session if this is the first web viewer
    if (current_count == 0) {
        if (!StartRemoteDesktop(device_id)) {
            SendText(ws_ptr, BuildJsonResponse("connect_result", false, "Failed to start remote desktop"));
            return;
        }
    }

    // Get screen dimensions from device info cache (may not be available yet)
    int width = 0, height = 0;
    {
        std::lock_guard<std::mutex> lock(m_DeviceCacheMutex);
        auto it = m_DeviceCache.find(device_id);
        if (it != m_DeviceCache.end()) {
            width = it->second->screen_width;
            height = it->second->screen_height;
        }
    }

    // Build success response
    Json::Value res;
    res["cmd"] = "connect_result";
    res["ok"] = true;
    // Only include dimensions if we have valid cached values
    // Otherwise, client will wait for resolution_changed message
    if (width > 0 && height > 0) {
        res["width"] = width;
        res["height"] = height;
    }
    res["algorithm"] = "h264";

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    SendText(ws_ptr, Json::writeString(builder, res));

    // Send cached keyframe if available
    {
        std::lock_guard<std::mutex> lock(m_DeviceCacheMutex);
        auto it = m_DeviceCache.find(device_id);
        if (it != m_DeviceCache.end() && !it->second->keyframe_cache.empty()) {
            std::lock_guard<std::mutex> cache_lock(it->second->cache_mutex);
            auto packet = BuildFramePacket(device_id, true,
                it->second->keyframe_cache.data(),
                it->second->keyframe_cache.size());
            SendBinary(ws_ptr, packet.data(), packet.size());
        }
    }
}

void CWebService::HandleDisconnect(void* ws_ptr, const std::string& token) {
    std::string username, role;
    if (!ValidateToken(token, username, role)) {
        SendText(ws_ptr, BuildJsonResponse("disconnect_result", false, "Invalid token"));
        return;
    }

    // Get the device_id before clearing watch state
    uint64_t device_id = 0;
    {
        std::lock_guard<std::mutex> lock(m_ClientsMutex);
        auto it = m_Clients.find(ws_ptr);
        if (it != m_Clients.end()) {
            device_id = it->second.watch_device_id;
            it->second.watch_device_id = 0;
        }
    }

    // Close remote desktop if this was the last viewer
    if (device_id > 0) {
        StopRemoteDesktop(device_id);
    }

    SendText(ws_ptr, BuildJsonResponse("disconnect_result", true));
}

void CWebService::HandlePing(void* ws_ptr, const std::string& token) {
    Json::Value res;
    res["cmd"] = "pong";
    res["time"] = (Json::Int64)time(nullptr);

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    SendText(ws_ptr, Json::writeString(builder, res));
}

void CWebService::HandleMouse(void* ws_ptr, const std::string& msg) {
    // Parse JSON
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(msg, root)) {
        return;
    }

    std::string token = root.get("token", "").asString();
    std::string type = root.get("type", "").asString();
    int x = root.get("x", 0).asInt();
    int y = root.get("y", 0).asInt();
    int button = root.get("button", 0).asInt();
    int delta = root.get("delta", 0).asInt();

    // Validate token
    std::string username, role;
    if (!ValidateToken(token, username, role)) {
        return;
    }

    // Get device being watched
    uint64_t device_id = 0;
    {
        std::lock_guard<std::mutex> lock(m_ClientsMutex);
        auto it = m_Clients.find(ws_ptr);
        if (it != m_Clients.end()) {
            device_id = it->second.watch_device_id;
            it->second.last_activity = (uint64_t)time(nullptr);
        }
    }

    if (device_id == 0) return;

    // Get screen context (not main context!)
    CONTEXT_OBJECT* ctx = GetScreenContext(device_id);
    if (!ctx) {
        return;
    }

    // Build MSG64 structure
    MSG64 msg64;
    memset(&msg64, 0, sizeof(MSG64));
    msg64.pt.x = x;
    msg64.pt.y = y;
    msg64.lParam = MAKELPARAM(x, y);
    msg64.time = GetTickCount();

    // Map type and button to Windows message
    if (type == "down") {
        if (button == 0) {
            msg64.message = WM_LBUTTONDOWN;
            msg64.wParam = MK_LBUTTON;
        } else if (button == 1) {
            msg64.message = WM_MBUTTONDOWN;
            msg64.wParam = MK_MBUTTON;
        } else if (button == 2) {
            msg64.message = WM_RBUTTONDOWN;
            msg64.wParam = MK_RBUTTON;
        }
    } else if (type == "up") {
        if (button == 0) {
            msg64.message = WM_LBUTTONUP;
        } else if (button == 1) {
            msg64.message = WM_MBUTTONUP;
        } else if (button == 2) {
            msg64.message = WM_RBUTTONUP;
        }
    } else if (type == "move") {
        msg64.message = WM_MOUSEMOVE;
    } else if (type == "wheel") {
        msg64.message = WM_MOUSEWHEEL;
        // WM_MOUSEWHEEL: HIWORD(wParam) = wheel delta, LOWORD(wParam) = key flags
        // Normalize: browser delta is usually ±100+, Windows expects ±120
        short wheelDelta = (short)(delta > 0 ? -120 : (delta < 0 ? 120 : 0));
        msg64.wParam = MAKEWPARAM(0, wheelDelta);
    } else if (type == "dblclick") {
        if (button == 0) {
            msg64.message = WM_LBUTTONDBLCLK;
            msg64.wParam = MK_LBUTTON;
        } else if (button == 2) {
            msg64.message = WM_RBUTTONDBLCLK;
            msg64.wParam = MK_RBUTTON;
        }
    } else {
        return;  // Unknown type
    }

    // Send command to device
    const int length = sizeof(MSG64) + 1;
    BYTE szData[length + 4];
    szData[0] = COMMAND_SCREEN_CONTROL;
    memcpy(szData + 1, &msg64, sizeof(MSG64));
    ctx->Send2Client(szData, length);
}

void CWebService::HandleKey(void* ws_ptr, const std::string& msg) {
    // Parse JSON
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(msg, root)) {
        return;
    }

    std::string token = root.get("token", "").asString();
    int keyCode = root.get("keyCode", 0).asInt();
    bool isDown = root.get("down", false).asBool();
    bool altKey = root.get("alt", false).asBool();

    // Filter Windows keys (same as MFC version)
    if (keyCode == VK_LWIN || keyCode == VK_RWIN) {
        return;
    }

    // Validate token
    std::string username, role;
    if (!ValidateToken(token, username, role)) {
        return;
    }

    // Get device being watched
    uint64_t device_id = 0;
    {
        std::lock_guard<std::mutex> lock(m_ClientsMutex);
        auto it = m_Clients.find(ws_ptr);
        if (it != m_Clients.end()) {
            device_id = it->second.watch_device_id;
            it->second.last_activity = (uint64_t)time(nullptr);
        }
    }

    if (device_id == 0) return;

    // Get screen context
    CONTEXT_OBJECT* ctx = GetScreenContext(device_id);
    if (!ctx) {
        return;
    }

    // Build MSG64 structure
    MSG64 msg64;
    memset(&msg64, 0, sizeof(MSG64));

    // Use WM_SYSKEYDOWN/UP for Alt combinations (same as MFC version)
    if (altKey) {
        msg64.message = isDown ? WM_SYSKEYDOWN : WM_SYSKEYUP;
    } else {
        msg64.message = isDown ? WM_KEYDOWN : WM_KEYUP;
    }
    msg64.wParam = keyCode;
    msg64.time = GetTickCount();

    // Build lParam for keyboard message:
    // bits 0-15: repeat count (1)
    // bits 16-23: scan code
    // bit 24: extended key flag
    // bit 29: context code (1 if Alt is pressed)
    // bit 30: previous key state (1 for keyup)
    // bit 31: transition state (1 for keyup)
    UINT scanCode = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);

    // Extended keys: arrows, insert, delete, home, end, page up/down, numpad enter, etc.
    bool isExtended = (keyCode >= VK_PRIOR && keyCode <= VK_DOWN) ||  // Page Up/Down, End, Home, Arrows
                      keyCode == VK_INSERT || keyCode == VK_DELETE ||
                      keyCode == VK_NUMLOCK || keyCode == VK_RCONTROL || keyCode == VK_RMENU ||
                      keyCode == VK_APPS;

    LPARAM lParam = 1;  // repeat count = 1
    lParam |= (scanCode & 0xFF) << 16;
    if (isExtended) lParam |= (1 << 24);
    if (altKey) lParam |= (1 << 29);  // context code for Alt
    if (!isDown) lParam |= (3UL << 30);  // bit 30 and 31 set for key up

    msg64.lParam = lParam;

    // Send command to device
    const int length = sizeof(MSG64) + 1;
    BYTE szData[length + 4];
    szData[0] = COMMAND_SCREEN_CONTROL;
    memcpy(szData + 1, &msg64, sizeof(MSG64));
    ctx->Send2Client(szData, length);
}

void CWebService::HandleRdpReset(void* ws_ptr, const std::string& token) {
    std::string username, role;
    if (!ValidateToken(token, username, role)) {
        SendText(ws_ptr, BuildJsonResponse("rdp_reset_result", false, "Invalid token"));
        return;
    }

    // Get the device being watched by this client
    uint64_t device_id = 0;
    {
        std::lock_guard<std::mutex> lock(m_ClientsMutex);
        auto it = m_Clients.find(ws_ptr);
        if (it != m_Clients.end()) {
            device_id = it->second.watch_device_id;
        }
    }

    if (device_id == 0) {
        SendText(ws_ptr, BuildJsonResponse("rdp_reset_result", false, "No device connected"));
        return;
    }

    // Get screen context (not main context!)
    CONTEXT_OBJECT* ctx = GetScreenContext(device_id);
    if (!ctx) {
        Mprintf("[WebService] HandleRdpReset: No screen context for device %llu\n", device_id);
        SendText(ws_ptr, BuildJsonResponse("rdp_reset_result", false, "No active screen session"));
        return;
    }

    // Send CMD_RESTORE_CONSOLE command to client
    BYTE bToken = CMD_RESTORE_CONSOLE;
    if (ctx->Send2Client(&bToken, 1)) {
        Mprintf("[WebService] Sent RDP reset command to device %llu\n", device_id);
        SendText(ws_ptr, BuildJsonResponse("rdp_reset_result", true));
    } else {
        SendText(ws_ptr, BuildJsonResponse("rdp_reset_result", false, "Failed to send command"));
    }
}

//////////////////////////////////////////////////////////////////////////
// Token Management (delegated to WebServiceAuth module)
//////////////////////////////////////////////////////////////////////////

std::string CWebService::GenerateToken(const std::string& username, const std::string& role) {
    return WSAuth::GenerateToken(username, role, m_nTokenExpireSeconds);
}

bool CWebService::ValidateToken(const std::string& token, std::string& username, std::string& role) {
    return WSAuth::ValidateToken(token, username, role);
}

//////////////////////////////////////////////////////////////////////////
// Client Management
//////////////////////////////////////////////////////////////////////////

void CWebService::RegisterClient(void* ws_ptr, const std::string& client_ip) {
    std::lock_guard<std::mutex> lock(m_ClientsMutex);
    WebClient client;
    client.client_ip = client_ip;
    client.connected_at = (uint64_t)time(nullptr);
    client.last_activity = client.connected_at;  // Initialize for heartbeat
    m_Clients[ws_ptr] = client;
}

void CWebService::UnregisterClient(void* ws_ptr) {
    uint64_t device_id = 0;

    // Get device_id and remove client
    {
        std::lock_guard<std::mutex> lock(m_ClientsMutex);
        auto it = m_Clients.find(ws_ptr);
        if (it != m_Clients.end()) {
            device_id = it->second.watch_device_id;
            m_Clients.erase(it);
        }
    }

    // Close remote desktop if this was the last viewer
    if (device_id > 0) {
        StopRemoteDesktop(device_id);
    }
}

WebClient* CWebService::FindClient(void* ws_ptr) {
    auto it = m_Clients.find(ws_ptr);
    return (it != m_Clients.end()) ? &it->second : nullptr;
}

int CWebService::GetWebClientCount(uint64_t device_id) {
    std::lock_guard<std::mutex> lock(m_ClientsMutex);
    int count = 0;
    for (const auto& [ptr, client] : m_Clients) {
        if (client.watch_device_id == device_id) {
            count++;
        }
    }
    return count;
}

//////////////////////////////////////////////////////////////////////////
// Rate Limiting
//////////////////////////////////////////////////////////////////////////

bool CWebService::CheckRateLimit(const std::string& ip) {
    std::lock_guard<std::mutex> lock(m_LoginMutex);
    auto it = m_LoginAttempts.find(ip);
    if (it == m_LoginAttempts.end()) return true;
    return time(nullptr) >= it->second.locked_until;
}

void CWebService::RecordFailedLogin(const std::string& ip) {
    std::lock_guard<std::mutex> lock(m_LoginMutex);
    auto& attempt = m_LoginAttempts[ip];
    attempt.failed_count++;
    if (attempt.failed_count >= 3) {
        attempt.locked_until = time(nullptr) + 3600;  // Lock for 1 hour
        attempt.failed_count = 0;
        Mprintf("[WebService] IP %s locked for 1 hour due to failed login attempts\n", ip.c_str());
    }
}

void CWebService::RecordSuccessLogin(const std::string& ip) {
    std::lock_guard<std::mutex> lock(m_LoginMutex);
    m_LoginAttempts.erase(ip);
}

//////////////////////////////////////////////////////////////////////////
// Password Verification (delegated to WebServiceAuth module)
//////////////////////////////////////////////////////////////////////////

bool CWebService::VerifyPassword(const std::string& input, const WebUser& user) {
    return WSAuth::VerifyPassword(input, user.password_hash, user.salt);
}

std::string CWebService::ComputeHash(const std::string& input) {
    return WSAuth::ComputeSHA256(input);
}

//////////////////////////////////////////////////////////////////////////
// JSON Helpers
//////////////////////////////////////////////////////////////////////////

std::string CWebService::BuildJsonResponse(const std::string& cmd, bool ok, const std::string& msg) {
    Json::Value res;
    res["cmd"] = cmd;
    res["ok"] = ok;
    if (!msg.empty()) {
        res["msg"] = msg;
    }

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, res);
}

std::string CWebService::BuildDeviceListJson() {
    Json::Value res;
    res["cmd"] = "device_list";
    res["devices"] = Json::Value(Json::arrayValue);

    if (m_pParentDlg) {
        // Access device list with lock
        EnterCriticalSection(&m_pParentDlg->m_cs);
        for (context* ctx : m_pParentDlg->m_HostList) {
            if (!ctx || !ctx->IsLogin()) continue;

            Json::Value device;
            // Use string for ID to avoid JavaScript number precision loss
            device["id"] = std::to_string(ctx->GetClientID());

            CString name = ctx->GetClientData(ONLINELIST_COMPUTER_NAME);
            device["name"] = AnsiToUtf8(name);

            CString ip = ctx->GetClientData(ONLINELIST_IP);
            device["ip"] = AnsiToUtf8(ip);

            CString os = ctx->GetClientData(ONLINELIST_OS);
            device["os"] = AnsiToUtf8(os);

            CString location = ctx->GetClientData(ONLINELIST_LOCATION);
            device["location"] = AnsiToUtf8(location);

            CString rtt = ctx->GetClientData(ONLINELIST_PING);
            device["rtt"] = AnsiToUtf8(rtt);

            CString version = ctx->GetClientData(ONLINELIST_VERSION);
            device["version"] = AnsiToUtf8(version);

            CString activeWindow = ctx->GetClientData(ONLINELIST_LOGINTIME);
            device["activeWindow"] = AnsiToUtf8(activeWindow);
            device["online"] = true;

            // Get screen info from client's reported resolution
            // Format: "n:MxN" where n=monitor count, M=width, N=height
            CString resolution = ctx->GetAdditionalData(RES_RESOLUTION);
            if (!resolution.IsEmpty()) {
                device["screen"] = AnsiToUtf8(resolution);  // e.g. "2:3840x1080"
            }

            res["devices"].append(device);
        }
        LeaveCriticalSection(&m_pParentDlg->m_cs);
    }

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, res);
}

//////////////////////////////////////////////////////////////////////////
// WebSocket Send Helpers
//////////////////////////////////////////////////////////////////////////

void CWebService::SendText(void* ws_ptr, const std::string& text) {
    if (!ws_ptr || m_bStopping) return;
    ws::Connection* conn = (ws::Connection*)ws_ptr;
    if (!conn->isClosed()) {
        conn->send(text);
    }
}

void CWebService::SendBinary(void* ws_ptr, const uint8_t* data, size_t len) {
    if (!ws_ptr || !data || len == 0 || m_bStopping) return;
    ws::Connection* conn = (ws::Connection*)ws_ptr;
    if (!conn->isClosed()) {
        conn->sendBinary(data, len);
    }
}

//////////////////////////////////////////////////////////////////////////
// Frame Broadcasting
//////////////////////////////////////////////////////////////////////////

std::vector<uint8_t> CWebService::BuildFramePacket(uint64_t device_id, bool is_keyframe,
                                                    const uint8_t* data, size_t len) {
    // Packet format: [DeviceID:4][FrameType:1][DataLen:4][H264Data:N]
    std::vector<uint8_t> packet;
    packet.reserve(9 + len);

    // DeviceID (4 bytes, little-endian, truncated to 32-bit for JS compatibility)
    uint32_t id32 = (uint32_t)(device_id & 0xFFFFFFFF);
    packet.push_back((id32 >> 0) & 0xFF);
    packet.push_back((id32 >> 8) & 0xFF);
    packet.push_back((id32 >> 16) & 0xFF);
    packet.push_back((id32 >> 24) & 0xFF);

    // FrameType (1 byte): 0=P, 1=IDR
    packet.push_back(is_keyframe ? 1 : 0);

    // DataLen (4 bytes, little-endian)
    uint32_t data_len = (uint32_t)len;
    packet.push_back((data_len >> 0) & 0xFF);
    packet.push_back((data_len >> 8) & 0xFF);
    packet.push_back((data_len >> 16) & 0xFF);
    packet.push_back((data_len >> 24) & 0xFF);

    // H264 Data
    packet.insert(packet.end(), data, data + len);

    return packet;
}

void CWebService::BroadcastFrame(uint64_t device_id, const uint8_t* data, size_t len, bool is_keyframe) {
    if (!data || len == 0 || m_bStopping) return;

    // Build packet once
    auto packet = BuildFramePacket(device_id, is_keyframe, data, len);

    // Broadcast to all watching clients
    std::lock_guard<std::mutex> lock(m_ClientsMutex);
    for (auto& [ws_ptr, client] : m_Clients) {
        if (client.watch_device_id == device_id) {
            SendBinary(ws_ptr, packet.data(), packet.size());
        }
    }
}

void CWebService::CacheKeyframe(uint64_t device_id, const uint8_t* data, size_t len) {
    if (!data || len == 0 || m_bStopping) return;

    std::lock_guard<std::mutex> lock(m_DeviceCacheMutex);
    auto it = m_DeviceCache.find(device_id);
    if (it == m_DeviceCache.end()) {
        m_DeviceCache[device_id] = std::make_shared<WebDeviceInfo>();
        it = m_DeviceCache.find(device_id);
    }

    std::lock_guard<std::mutex> cache_lock(it->second->cache_mutex);
    it->second->keyframe_cache.assign(data, data + len);
}

void CWebService::BroadcastH264Frame(uint64_t device_id, const uint8_t* data, size_t len) {
    // The data is already a complete packet: [DeviceID:4][FrameType:1][DataLen:4][H264Data:N]
    if (!data || len < 9 || m_bStopping) return;

    // Broadcast to all watching clients
    std::lock_guard<std::mutex> lock(m_ClientsMutex);
    int sent_count = 0;
    for (auto& [ws_ptr, client] : m_Clients) {
        if (client.watch_device_id == device_id) {
            SendBinary(ws_ptr, data, len);
            sent_count++;
        }
    }
    // Cache keyframe (check FrameType byte at offset 4)
    if (data[4] == 1) {  // IDR frame
        CacheKeyframe(device_id, data, len);
    }
}

void CWebService::NotifyResolutionChange(uint64_t device_id, int width, int height) {
    if (m_bStopping) return;

    // Update cache
    {
        std::lock_guard<std::mutex> lock(m_DeviceCacheMutex);
        auto it = m_DeviceCache.find(device_id);
        if (it == m_DeviceCache.end()) {
            m_DeviceCache[device_id] = std::make_shared<WebDeviceInfo>();
            it = m_DeviceCache.find(device_id);
        }
        it->second->screen_width = width;
        it->second->screen_height = height;
    }

    // Notify watching clients
    Json::Value res;
    res["cmd"] = "resolution_changed";
    res["id"] = device_id;
    res["width"] = width;
    res["height"] = height;

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string json = Json::writeString(builder, res);

    std::lock_guard<std::mutex> lock(m_ClientsMutex);
    for (auto& [ws_ptr, client] : m_Clients) {
        if (client.watch_device_id == device_id) {
            SendText(ws_ptr, json);
        }
    }
}

bool CWebService::StartRemoteDesktop(uint64_t device_id) {
    if (!m_pParentDlg) return false;

    context* ctx = m_pParentDlg->FindHost(device_id);
    if (!ctx) return false;

    // Close any existing remote desktop for this device first
    // This prevents duplicate dialogs when user reconnects quickly
    m_pParentDlg->CloseRemoteDesktopByClientID(device_id);

    // Mark as web-triggered (dialog should be hidden)
    {
        std::lock_guard<std::mutex> lock(m_WebTriggeredMutex);
        m_WebTriggeredDevices.insert(device_id);
    }

    // Send COMMAND_SCREEN_SPY with H264 algorithm
    // Format: [COMMAND_SCREEN_SPY:1][DXGI:1][Algorithm:1][MultiScreen:1]
    BYTE bToken[32] = { 0 };
    bToken[0] = COMMAND_SCREEN_SPY;
    bToken[1] = 0;  // DXGI mode: 0=GDI
    bToken[2] = ALGORITHM_H264;  // H264 algorithm
    bToken[3] = 1;  // Multi-screen: true

    return ctx->Send2Client(bToken, sizeof(bToken)) != FALSE;
}

void CWebService::StopRemoteDesktop(uint64_t device_id) {
    if (!m_pParentDlg) return;

    // Check if any other web clients are watching this device
    int watchingCount = 0;
    {
        std::lock_guard<std::mutex> lock(m_ClientsMutex);
        for (const auto& [ws_ptr, client] : m_Clients) {
            if (client.watch_device_id == device_id) {
                watchingCount++;
            }
        }
    }

    // If no more web clients watching, close the remote desktop
    if (watchingCount == 0) {
        ClearWebTriggered(device_id);
        m_pParentDlg->CloseRemoteDesktopByClientID(device_id);
    }
}

//////////////////////////////////////////////////////////////////////////
// Screen Context Registry (for mouse/keyboard control)
//////////////////////////////////////////////////////////////////////////

void CWebService::RegisterScreenContext(uint64_t device_id, CONTEXT_OBJECT* ctx) {
    if (!m_bRunning) return;
    std::lock_guard<std::mutex> lock(m_ScreenContextsMutex);
    m_ScreenContexts[device_id] = ctx;
    Mprintf("[WebService] Registered screen context for device %llu\n", device_id);
}

void CWebService::UnregisterScreenContext(uint64_t device_id) {
    if (!m_bRunning) return;
    std::lock_guard<std::mutex> lock(m_ScreenContextsMutex);
    m_ScreenContexts.erase(device_id);
    Mprintf("[WebService] Unregistered screen context for device %llu\n", device_id);
}

CONTEXT_OBJECT* CWebService::GetScreenContext(uint64_t device_id) {
    std::lock_guard<std::mutex> lock(m_ScreenContextsMutex);
    auto it = m_ScreenContexts.find(device_id);
    return (it != m_ScreenContexts.end()) ? it->second : nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Device Events
//////////////////////////////////////////////////////////////////////////

void CWebService::MarkDeviceOnline(uint64_t device_id) {
    std::lock_guard<std::mutex> lock(m_DirtyDevicesMutex);
    m_OnlineDevices.insert(device_id);
    m_OfflineDevices.erase(device_id);  // Cancel pending offline if any
}

void CWebService::MarkDeviceOffline(uint64_t device_id) {
    std::lock_guard<std::mutex> lock(m_DirtyDevicesMutex);
    m_OfflineDevices.insert(device_id);
    m_OnlineDevices.erase(device_id);  // Cancel pending online if any
}

void CWebService::FlushDeviceChanges() {
    if (!m_bRunning || m_bStopping) return;

    // Collect changes under lock
    std::set<uint64_t> online, offline;
    {
        std::lock_guard<std::mutex> lock(m_DirtyDevicesMutex);
        if (m_OnlineDevices.empty() && m_OfflineDevices.empty()) {
            return;  // No changes
        }
        online = std::move(m_OnlineDevices);
        offline = std::move(m_OfflineDevices);
        m_OnlineDevices.clear();
        m_OfflineDevices.clear();
    }

    // Notify clients watching offline devices
    if (!offline.empty()) {
        std::lock_guard<std::mutex> lock(m_ClientsMutex);
        for (auto& [ws_ptr, client] : m_Clients) {
            if (offline.count(client.watch_device_id)) {
                Json::Value res;
                res["cmd"] = "device_offline";
                res["id"] = std::to_string(client.watch_device_id);
                Json::StreamWriterBuilder builder;
                builder["indentation"] = "";
                SendText(ws_ptr, Json::writeString(builder, res));
                client.watch_device_id = 0;
            }
        }
    }

    // Clear offline devices from cache
    {
        std::lock_guard<std::mutex> lock(m_DeviceCacheMutex);
        for (uint64_t id : offline) {
            m_DeviceCache.erase(id);
        }
    }

    // Build and broadcast devices_changed message
    Json::Value msg;
    msg["cmd"] = "devices_changed";
    msg["online"] = Json::Value(Json::arrayValue);
    msg["offline"] = Json::Value(Json::arrayValue);
    for (uint64_t id : online) {
        msg["online"].append(std::to_string(id));
    }
    for (uint64_t id : offline) {
        msg["offline"].append(std::to_string(id));
    }

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string json = Json::writeString(builder, msg);

    // Broadcast to all authenticated clients
    std::lock_guard<std::mutex> lock(m_ClientsMutex);
    for (const auto& [ws_ptr, client] : m_Clients) {
        if (!client.token.empty()) {
            SendText(ws_ptr, json);
        }
    }
}

bool CWebService::IsWebTriggered(uint64_t device_id) {
    std::lock_guard<std::mutex> lock(m_WebTriggeredMutex);
    return m_WebTriggeredDevices.find(device_id) != m_WebTriggeredDevices.end();
}

void CWebService::ClearWebTriggered(uint64_t device_id) {
    std::lock_guard<std::mutex> lock(m_WebTriggeredMutex);
    m_WebTriggeredDevices.erase(device_id);
}

void CWebService::NotifyDeviceUpdate(uint64_t device_id, const std::string& rtt, const std::string& activeWindow) {
    if (!m_bRunning || m_bStopping) return;

    // Build update message
    Json::Value msg;
    msg["cmd"] = "device_update";
    msg["id"] = std::to_string(device_id);
    msg["rtt"] = rtt;
    msg["activeWindow"] = activeWindow;

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string json_str = Json::writeString(builder, msg);

    // Broadcast to all authenticated clients
    std::lock_guard<std::mutex> lock(m_ClientsMutex);
    for (const auto& [ws_ptr, client] : m_Clients) {
        if (!client.token.empty()) {
            SendText(ws_ptr, json_str);
        }
    }
}
