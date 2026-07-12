#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <thread>
#include <memory>
#include <random>
#include <ctime>

// Forward declarations
class context;
class CMy2015RemoteDlg;
class CONTEXT_OBJECT;

// Web client state
struct WebClient {
    std::string token;
    std::string username;
    std::string role;           // "admin" | "viewer"
    uint64_t watch_device_id;   // 0 = not watching any device
    uint64_t connected_at;
    uint64_t last_activity;     // Last message/pong received (for heartbeat timeout)
    std::string client_ip;

    WebClient() : watch_device_id(0), connected_at(0), last_activity(0) {}
};

// Login attempt tracking for rate limiting
struct LoginAttempt {
    int failed_count;
    time_t locked_until;

    LoginAttempt() : failed_count(0), locked_until(0) {}
};

// Web user account
struct WebUser {
    std::string username;
    std::string password_hash;  // SHA256(password + salt)
    std::string salt;
    std::string role;           // "admin" | "viewer"
};

// Device info for web clients
struct WebDeviceInfo {
    uint64_t id;
    std::string name;
    std::string ip;
    std::string os;
    int screen_width;
    int screen_height;
    bool online;

    // Keyframe cache for new web clients
    std::vector<uint8_t> keyframe_cache;
    std::mutex cache_mutex;

    WebDeviceInfo() : id(0), screen_width(0), screen_height(0), online(false) {}
};

// Main Web Service class
class CWebService {
public:
    static CWebService& Instance();

    // Lifecycle
    bool Start(int port = 8080);
    void Stop();
    bool IsRunning() const;

    // Set parent dialog for device list access
    void SetParentDlg(CMy2015RemoteDlg* pDlg) { m_pParentDlg = pDlg; }

    // Set admin password (use master password)
    void SetAdminPassword(const std::string& password);

    // Device management (called from main app)
    void MarkDeviceOnline(uint64_t device_id);
    void MarkDeviceOffline(uint64_t device_id);
    void FlushDeviceChanges();  // Called by timer to batch-notify web clients

    // H264 frame broadcasting (called from ScreenSpyDlg)
    void BroadcastFrame(uint64_t device_id, const uint8_t* data, size_t len, bool is_keyframe);
    void BroadcastH264Frame(uint64_t device_id, const uint8_t* data, size_t len);
    void CacheKeyframe(uint64_t device_id, const uint8_t* data, size_t len);

    // Resolution change notification
    void NotifyResolutionChange(uint64_t device_id, int width, int height);

    // Get count of web clients watching a device
    int GetWebClientCount(uint64_t device_id);

    // Start remote desktop session for web viewing
    bool StartRemoteDesktop(uint64_t device_id);

    // Stop remote desktop session
    void StopRemoteDesktop(uint64_t device_id);

private:
    CWebService();
    ~CWebService();
    CWebService(const CWebService&) = delete;
    CWebService& operator=(const CWebService&) = delete;

    // Server thread
    void ServerThread(int port);

    // Signaling handlers
    void HandleLogin(void* ws_ptr, const std::string& msg, const std::string& client_ip);
    void HandleGetDevices(void* ws_ptr, const std::string& token);
    void HandleConnect(void* ws_ptr, const std::string& token, uint64_t device_id);
    void HandleDisconnect(void* ws_ptr, const std::string& token);
    void HandlePing(void* ws_ptr, const std::string& token);
    void HandleMouse(void* ws_ptr, const std::string& msg);
    void HandleKey(void* ws_ptr, const std::string& msg);
    void HandleRdpReset(void* ws_ptr, const std::string& token);

    // Token management
    std::string GenerateToken(const std::string& username, const std::string& role);
    bool ValidateToken(const std::string& token, std::string& username, std::string& role);

    // Client management
    void RegisterClient(void* ws_ptr, const std::string& client_ip);
    void UnregisterClient(void* ws_ptr);
    WebClient* FindClient(void* ws_ptr);

    // Rate limiting
    bool CheckRateLimit(const std::string& ip);
    void RecordFailedLogin(const std::string& ip);
    void RecordSuccessLogin(const std::string& ip);

    // JSON helpers
    std::string BuildJsonResponse(const std::string& cmd, bool ok, const std::string& msg = "");
    std::string BuildDeviceListJson();

    // Password verification
    bool VerifyPassword(const std::string& input, const WebUser& user);
    std::string ComputeHash(const std::string& input);

    // Send to WebSocket
    void SendText(void* ws_ptr, const std::string& text);
    void SendBinary(void* ws_ptr, const uint8_t* data, size_t len);

    // Build binary frame packet
    std::vector<uint8_t> BuildFramePacket(uint64_t device_id, bool is_keyframe,
                                          const uint8_t* data, size_t len);

private:
    // Server state
    std::thread m_ServerThread;
    std::thread m_HeartbeatThread;  // Heartbeat checker thread
    std::atomic<bool> m_bRunning;
    std::atomic<bool> m_bStopping;
    void* m_pServer;  // ws::Server*

    // Heartbeat settings
    static const int HEARTBEAT_INTERVAL_SEC = 30;   // Send ping every 30 seconds
    static const int HEARTBEAT_TIMEOUT_SEC = 90;    // Disconnect if no activity for 90 seconds

    // Heartbeat thread function
    void HeartbeatThread();

    // Parent dialog for device access
    CMy2015RemoteDlg* m_pParentDlg;

    // Web clients: ws_ptr -> WebClient
    std::map<void*, WebClient> m_Clients;
    std::mutex m_ClientsMutex;

    // Device keyframe cache: device_id -> cache
    std::map<uint64_t, std::shared_ptr<WebDeviceInfo>> m_DeviceCache;
    std::mutex m_DeviceCacheMutex;

    // Login rate limiting: ip -> LoginAttempt
    std::map<std::string, LoginAttempt> m_LoginAttempts;
    std::mutex m_LoginMutex;

    // User accounts (loaded from config)
    std::vector<WebUser> m_Users;

    // Token secret key (generated on startup)
    std::string m_SecretKey;

    // Config
    int m_nMaxClientsPerDevice;
    int m_nTokenExpireSeconds;
    bool m_bHideWebSessions;  // Whether to hide web-triggered dialogs (default: true)
    std::string m_PayloadsDir;  // Directory for file downloads (Payloads/)

    // Web-triggered sessions (should be hidden)
    std::set<uint64_t> m_WebTriggeredDevices;
    std::mutex m_WebTriggeredMutex;

    // Dirty device tracking for batch notifications
    std::set<uint64_t> m_OnlineDevices;   // Devices that came online since last flush
    std::set<uint64_t> m_OfflineDevices;  // Devices that went offline since last flush
    std::mutex m_DirtyDevicesMutex;

public:
    // Check if a device session was triggered by web (should be hidden)
    bool IsWebTriggered(uint64_t device_id);
    void ClearWebTriggered(uint64_t device_id);

    // Config accessors
    void SetHideWebSessions(bool hide) { m_bHideWebSessions = hide; }
    bool GetHideWebSessions() const { return m_bHideWebSessions; }

    // Real-time device updates
    void NotifyDeviceUpdate(uint64_t device_id, const std::string& rtt, const std::string& activeWindow);

    // Screen context registry (for mouse/keyboard control)
    void RegisterScreenContext(uint64_t device_id, CONTEXT_OBJECT* ctx);
    void UnregisterScreenContext(uint64_t device_id);
    CONTEXT_OBJECT* GetScreenContext(uint64_t device_id);

private:
    // Screen context registry: device_id -> ScreenManager's CONTEXT_OBJECT
    std::map<uint64_t, CONTEXT_OBJECT*> m_ScreenContexts;
    std::mutex m_ScreenContextsMutex;
};

// Global accessor
inline CWebService& WebService() { return CWebService::Instance(); }
