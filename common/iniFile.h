#pragma once

// 支持独立模式（用于单元测试，避免 MFC 依赖）
// 在包含此头文件前定义 INIFILE_STANDALONE 并提供 StringToVector、GET_FILEPATH
#ifndef INIFILE_STANDALONE
#include "common/commands.h"
#endif

#define YAMA_PATH			"Software\\YAMA"
#define CLIENT_PATH			GetRegistryName()

#define NO_CURRENTKEY 1

#if NO_CURRENTKEY
#include <wtsapi32.h>
#include <sddl.h>
#pragma comment(lib, "wtsapi32.lib")

#ifndef SAFE_CLOSE_HANDLE
#define SAFE_CLOSE_HANDLE(h) do{if((h)!=NULL&&(h)!=INVALID_HANDLE_VALUE){CloseHandle(h);(h)=NULL;}}while(0)
#endif

inline std::string GetExeDir()
{
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);

    char* lastSlash = strrchr(path, '\\');
    if (lastSlash) *lastSlash = '\0';

    CharLowerA(path);
    return path;
}

inline std::string GetExeHashStr()
{
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    CharLowerA(path);

    ULONGLONG hash = 14695981039346656037ULL;
    for (const char* p = path; *p; p++) {
        hash ^= (unsigned char)*p;
        hash *= 1099511628211ULL;
    }

    char result[17];
    sprintf_s(result, "%016llX", hash);
    return result;
}

static inline std::string GetRegistryName()
{
    static auto name = "Software\\" + GetExeHashStr();
    return name;
}

// 获取当前会话用户的注册表根键
// SYSTEM 进程无法使用 HKEY_CURRENT_USER，需要通过 HKEY_USERS\<SID> 访问
// 返回的 HKEY 需要调用者在使用完毕后调用 RegCloseKey 关闭
inline HKEY InitCurrentUserRegistryKey()
{
    HKEY hUserKey = NULL;
    // 获取当前进程的会话 ID
    DWORD sessionId = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &sessionId);

    // 如果在 Session 0（服务进程），需要找用户会话
    if (sessionId == 0) {
        // 优先控制台会话（本地登录）
        sessionId = WTSGetActiveConsoleSessionId();

        // 没有控制台会话，枚举找远程会话（mstsc 登录）
        if (sessionId == 0xFFFFFFFF) {
            WTS_SESSION_INFOA* pSessions = NULL;
            DWORD count = 0;
            if (WTSEnumerateSessionsA(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessions, &count)) {
                for (DWORD i = 0; i < count; i++) {
                    if (pSessions[i].State == WTSActive && pSessions[i].SessionId != 0) {
                        sessionId = pSessions[i].SessionId;
                        break;
                    }
                }
                WTSFreeMemory(pSessions);
            }
        }
    }

    // 如果仍然没有有效会话，回退
    if (sessionId == 0 || sessionId == 0xFFFFFFFF) {
        return HKEY_CURRENT_USER;
    }

    // 获取该会话的用户令牌
    HANDLE hUserToken = NULL;
    if (!WTSQueryUserToken(sessionId, &hUserToken)) {
        // 如果失败（可能不是服务进程），回退到 HKEY_CURRENT_USER
        return HKEY_CURRENT_USER;
    }

    // 获取令牌中的用户信息大小
    DWORD dwSize = 0;
    GetTokenInformation(hUserToken, TokenUser, NULL, 0, &dwSize);
    if (dwSize == 0) {
        SAFE_CLOSE_HANDLE(hUserToken);
        return HKEY_CURRENT_USER;
    }

    // 分配内存并获取用户信息
    TOKEN_USER* pTokenUser = (TOKEN_USER*)malloc(dwSize);
    if (!pTokenUser) {
        SAFE_CLOSE_HANDLE(hUserToken);
        return HKEY_CURRENT_USER;
    }

    if (!GetTokenInformation(hUserToken, TokenUser, pTokenUser, dwSize, &dwSize)) {
        free(pTokenUser);
        SAFE_CLOSE_HANDLE(hUserToken);
        return HKEY_CURRENT_USER;
    }

    // 将 SID 转换为字符串
    LPSTR szSid = NULL;
    if (!ConvertSidToStringSidA(pTokenUser->User.Sid, &szSid)) {
        free(pTokenUser);
        SAFE_CLOSE_HANDLE(hUserToken);
        return HKEY_CURRENT_USER;
    }

    // 打开 HKEY_USERS\<SID>
    if (RegOpenKeyExA(HKEY_USERS, szSid, 0, KEY_READ | KEY_WRITE, &hUserKey) != ERROR_SUCCESS) {
        // 尝试只读方式
        if (RegOpenKeyExA(HKEY_USERS, szSid, 0, KEY_READ, &hUserKey) != ERROR_SUCCESS) {
            hUserKey = NULL;
        }
    }

    LocalFree(szSid);
    free(pTokenUser);
    SAFE_CLOSE_HANDLE(hUserToken);

    return hUserKey ? hUserKey : HKEY_CURRENT_USER;
}

// 获取当前会话用户的注册表根键（带缓存，线程安全）
// SYSTEM 进程无法使用 HKEY_CURRENT_USER，需要通过 HKEY_USERS\<SID> 访问
// 返回的 HKEY 由静态缓存管理，调用者不需要关闭
// 使用 C++11 magic statics 保证线程安全初始化
inline HKEY GetCurrentUserRegistryKey()
{
    static HKEY s_cachedKey = InitCurrentUserRegistryKey();
    return s_cachedKey;
}

// 检查是否需要关闭注册表根键
// 注意：GetCurrentUserRegistryKey() 返回的键现在是静态缓存的，不应关闭
inline void CloseUserRegistryKeyIfNeeded(HKEY hKey)
{
    // 静态缓存的键不关闭，由进程退出时自动清理
    (void)hKey;
}

#else
#define GetCurrentUserRegistryKey() HKEY_CURRENT_USER
#define CloseUserRegistryKeyIfNeeded(hKey)
#endif


// 配置读取类: 文件配置.
class config
{
private:
    char m_IniFilePath[_MAX_PATH] = { 0 };

public:
    virtual ~config() {}

    config(const std::string& path="")
    {
        if (path.length() == 0) {
            ::GetModuleFileNameA(NULL, m_IniFilePath, sizeof(m_IniFilePath));
            GET_FILEPATH(m_IniFilePath, "settings.ini");
        } else {
            strncpy_s(m_IniFilePath, sizeof(m_IniFilePath), path.c_str(), _TRUNCATE);
        }
    }

    virtual int GetInt(const std::string& MainKey, const std::string& SubKey, int nDef=0)
    {
        return ::GetPrivateProfileIntA(MainKey.c_str(), SubKey.c_str(), nDef, m_IniFilePath);
    }

    // 获取配置项中的第一个整数
    virtual int Get1Int(const std::string& MainKey, const std::string& SubKey, char ch=';', int nDef=0)
    {
        std::string s = GetStr(MainKey, SubKey, "");
        s = StringToVector(s, ch)[0];
        return s.empty() ? nDef : atoi(s.c_str());
    }

    virtual bool SetInt(const std::string& MainKey, const std::string& SubKey, int Data)
    {
        std::string strData = std::to_string(Data);
        BOOL ret = ::WritePrivateProfileStringA(MainKey.c_str(), SubKey.c_str(), strData.c_str(), m_IniFilePath);
        ::WritePrivateProfileStringA(NULL, NULL, NULL, m_IniFilePath);  // 刷新缓存
        return ret;
    }

    virtual std::string GetStr(const std::string& MainKey, const std::string& SubKey, const std::string& def = "")
    {
        char buf[4096] = { 0 };  // 增大缓冲区以支持较长的值（如 IP 列表）
        DWORD n = ::GetPrivateProfileStringA(MainKey.c_str(), SubKey.c_str(), def.c_str(), buf, sizeof(buf), m_IniFilePath);
        return std::string(buf);
    }

    virtual bool SetStr(const std::string& MainKey, const std::string& SubKey, const std::string& Data)
    {
        BOOL ret = ::WritePrivateProfileStringA(MainKey.c_str(), SubKey.c_str(), Data.c_str(), m_IniFilePath);
        ::WritePrivateProfileStringA(NULL, NULL, NULL, m_IniFilePath);  // 刷新缓存
        return ret;
    }

    virtual double GetDouble(const std::string& MainKey, const std::string& SubKey, double dDef = 0.0)
    {
        std::string val = GetStr(MainKey, SubKey);
        if (val.empty())
            return dDef;
        try {
            return std::stod(val);
        } catch (...) {
            return dDef;
        }
    }

    virtual bool SetDouble(const std::string& MainKey, const std::string& SubKey, double Data)
    {
        char buf[64];
        sprintf_s(buf, "%.6f", Data);
        return SetStr(MainKey, SubKey, buf);  // SetStr 已包含刷新
    }
};

// 配置读取类: 注册表配置（带键句柄缓存）
// 注意：缓存操作非线程安全，但竞态条件只会导致少量重复打开，不会崩溃
class iniFile : public config
{
private:
    HKEY m_hRootKey;
    std::string m_SubKeyPath;

    // 注册表键句柄缓存，避免频繁 RegOpenKeyEx/RegCloseKey
    mutable std::map<std::string, HKEY> m_keyCache;

    // 获取缓存的键句柄，如果不存在则打开并缓存
    HKEY GetCachedKey(const std::string& mainKey) const
    {
        std::string fullPath = m_SubKeyPath + "\\" + mainKey;

        auto it = m_keyCache.find(fullPath);
        if (it != m_keyCache.end()) {
            return it->second;
        }

        HKEY hKey = NULL;
        if (RegCreateKeyExA(m_hRootKey, fullPath.c_str(), 0, NULL, 0,
                            KEY_READ | KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
            m_keyCache[fullPath] = hKey;
            return hKey;
        }
        return NULL;
    }

public:
    ~iniFile()
    {
        // 关闭所有缓存的键句柄
        for (auto& pair : m_keyCache) {
            if (pair.second) {
                RegCloseKey(pair.second);
            }
        }
        m_keyCache.clear();
    }

    iniFile(const std::string& path = YAMA_PATH)
    {
        m_hRootKey = GetCurrentUserRegistryKey();
        m_SubKeyPath = path;
        if (path != YAMA_PATH) {
            static std::string workSpace = GetExeDir();
            SetStr("settings", "work_space", workSpace);
        }
    }

    // 禁用拷贝和移动（因为有缓存的句柄）
    iniFile(const iniFile&) = delete;
    iniFile& operator=(const iniFile&) = delete;
    iniFile(iniFile&&) = delete;
    iniFile& operator=(iniFile&&) = delete;

    // 写入整数，实际写为字符串
    bool SetInt(const std::string& MainKey, const std::string& SubKey, int Data) override
    {
        return SetStr(MainKey, SubKey, std::to_string(Data));
    }

    // 写入字符串（使用缓存的键句柄）
    bool SetStr(const std::string& MainKey, const std::string& SubKey, const std::string& Data) override
    {
        HKEY hKey = GetCachedKey(MainKey);
        if (!hKey)
            return false;

        return RegSetValueExA(hKey, SubKey.c_str(), 0, REG_SZ,
                              reinterpret_cast<const BYTE*>(Data.c_str()),
                              static_cast<DWORD>(Data.size() + 1)) == ERROR_SUCCESS;
    }

    // 读取字符串（使用缓存的键句柄）
    std::string GetStr(const std::string& MainKey, const std::string& SubKey, const std::string& def = "") override
    {
        HKEY hKey = GetCachedKey(MainKey);
        if (!hKey)
            return def;

        char buffer[512] = { 0 };
        DWORD dwSize = sizeof(buffer);
        DWORD dwType = REG_SZ;

        if (RegQueryValueExA(hKey, SubKey.c_str(), NULL, &dwType,
                             reinterpret_cast<LPBYTE>(buffer), &dwSize) == ERROR_SUCCESS &&
            dwType == REG_SZ) {
            return std::string(buffer);
        }
        return def;
    }

    // 读取整数，先从字符串中转换
    int GetInt(const std::string& MainKey, const std::string& SubKey, int defVal = 0) override
    {
        std::string val = GetStr(MainKey, SubKey);
        if (val.empty())
            return defVal;

        try {
            return std::stoi(val);
        } catch (...) {
            return defVal;
        }
    }

    // 清除键缓存（用于需要强制刷新的场景）
    void ClearKeyCache()
    {
        for (auto& pair : m_keyCache) {
            if (pair.second) {
                RegCloseKey(pair.second);
            }
        }
        m_keyCache.clear();
    }
};

// 配置读取类: 注册表二进制配置（带键句柄缓存）
class binFile : public config
{
private:
    HKEY m_hRootKey;
    std::string m_SubKeyPath;

    // 注册表键句柄缓存
    mutable std::map<std::string, HKEY> m_keyCache;

    // 获取缓存的键句柄
    HKEY GetCachedKey(const std::string& mainKey) const
    {
        std::string fullPath = m_SubKeyPath + "\\" + mainKey;

        auto it = m_keyCache.find(fullPath);
        if (it != m_keyCache.end()) {
            return it->second;
        }

        HKEY hKey = NULL;
        if (RegCreateKeyExA(m_hRootKey, fullPath.c_str(), 0, NULL, 0,
                            KEY_READ | KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
            m_keyCache[fullPath] = hKey;
            return hKey;
        }
        return NULL;
    }

public:
    ~binFile()
    {
        for (auto& pair : m_keyCache) {
            if (pair.second) {
                RegCloseKey(pair.second);
            }
        }
        m_keyCache.clear();
    }

    binFile(const std::string& path = CLIENT_PATH)
    {
        m_hRootKey = GetCurrentUserRegistryKey();
        m_SubKeyPath = path;
        if (path != YAMA_PATH) {
            static std::string workSpace = GetExeDir();
            SetStr("settings", "work_space", workSpace);
        }
    }

    // 禁用拷贝和移动（因为有缓存的句柄）
    binFile(const binFile&) = delete;
    binFile& operator=(const binFile&) = delete;
    binFile(binFile&&) = delete;
    binFile& operator=(binFile&&) = delete;

    // 写入整数（写为二进制）
    bool SetInt(const std::string& MainKey, const std::string& SubKey, int Data) override
    {
        return SetBinary(MainKey, SubKey, reinterpret_cast<const BYTE*>(&Data), sizeof(int));
    }

    // 写入字符串（以二进制方式）
    bool SetStr(const std::string& MainKey, const std::string& SubKey, const std::string& Data) override
    {
        return SetBinary(MainKey, SubKey, reinterpret_cast<const BYTE*>(Data.data()), static_cast<DWORD>(Data.size()));
    }

    // 读取字符串（从二进制数据转换）
    std::string GetStr(const std::string& MainKey, const std::string& SubKey, const std::string& def = "") override
    {
        std::vector<BYTE> buffer;
        if (!GetBinary(MainKey, SubKey, buffer))
            return def;

        return std::string(buffer.begin(), buffer.end());
    }

    // 读取整数（从二进制解析）
    int GetInt(const std::string& MainKey, const std::string& SubKey, int defVal = 0) override
    {
        std::vector<BYTE> buffer;
        if (!GetBinary(MainKey, SubKey, buffer) || buffer.size() < sizeof(int))
            return defVal;

        int value = 0;
        memcpy(&value, buffer.data(), sizeof(int));
        return value;
    }

    // 清除键缓存
    void ClearKeyCache()
    {
        for (auto& pair : m_keyCache) {
            if (pair.second) {
                RegCloseKey(pair.second);
            }
        }
        m_keyCache.clear();
    }

private:
    bool SetBinary(const std::string& MainKey, const std::string& SubKey, const BYTE* data, DWORD size)
    {
        HKEY hKey = GetCachedKey(MainKey);
        if (!hKey)
            return false;

        return RegSetValueExA(hKey, SubKey.c_str(), 0, REG_BINARY, data, size) == ERROR_SUCCESS;
    }

    bool GetBinary(const std::string& MainKey, const std::string& SubKey, std::vector<BYTE>& outData) const
    {
        HKEY hKey = GetCachedKey(MainKey);
        if (!hKey)
            return false;

        DWORD dwType = 0;
        DWORD dwSize = 0;
        if (RegQueryValueExA(hKey, SubKey.c_str(), NULL, &dwType, NULL, &dwSize) != ERROR_SUCCESS ||
            dwType != REG_BINARY) {
            return false;
        }

        outData.resize(dwSize);
        return RegQueryValueExA(hKey, SubKey.c_str(), NULL, NULL, outData.data(), &dwSize) == ERROR_SUCCESS;
    }
};