// IPWhitelist.h - IP 白名单管理 (单例)
// 用于多处共享白名单检查：连接限制、DLL 限流等

#pragma once
#include <string>
#include <set>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

class IPWhitelist {
public:
    static IPWhitelist& getInstance() {
        static IPWhitelist instance;
        return instance;
    }

    // 从配置字符串加载白名单 (格式: "192.168.1.1;10.0.0.1;172.16.0.0/24")
    void Load(const std::string& configValue) {
        AutoLock lock(m_Lock);
        m_IPs.clear();

        if (configValue.empty()) {
            return;
        }

        // 按分号分割
        size_t start = 0;
        size_t end = 0;
        while ((end = configValue.find(';', start)) != std::string::npos) {
            AddIPInternal(configValue.substr(start, end - start));
            start = end + 1;
        }
        // 最后一个 IP
        AddIPInternal(configValue.substr(start));
    }

    // 检查 IP 是否在白名单中
    bool IsWhitelisted(const std::string& ip) {
        // 本地地址始终白名单（无需加锁）
        if (ip == "127.0.0.1" || ip == "::1") {
            return true;
        }

        AutoLock lock(m_Lock);
        return m_IPs.find(ip) != m_IPs.end();
    }

    // 获取白名单数量
    size_t Count() {
        AutoLock lock(m_Lock);
        return m_IPs.size();
    }

    // 添加 IP 到白名单
    void AddIP(const std::string& ip) {
        AutoLock lock(m_Lock);
        AddIPInternal(ip);
    }

    // 从白名单移除 IP
    void RemoveIP(const std::string& ip) {
        AutoLock lock(m_Lock);
        m_IPs.erase(ip);
    }

    // 获取所有白名单 IP（用于显示）
    std::set<std::string> GetAll() {
        AutoLock lock(m_Lock);
        return m_IPs;
    }

    // 清空白名单
    void Clear() {
        AutoLock lock(m_Lock);
        m_IPs.clear();
    }

    // 导出为配置字符串
    std::string Export() {
        AutoLock lock(m_Lock);
        std::string result;
        for (const auto& ip : m_IPs) {
            if (!result.empty()) result += ";";
            result += ip;
        }
        return result;
    }

private:
    // RAII 锁，异常安全
#ifdef _WIN32
    class AutoLock {
    public:
        AutoLock(CRITICAL_SECTION& cs) : m_cs(cs) { EnterCriticalSection(&m_cs); }
        ~AutoLock() { LeaveCriticalSection(&m_cs); }
    private:
        CRITICAL_SECTION& m_cs;
    };
#else
    class AutoLock {
    public:
        AutoLock(pthread_mutex_t& mtx) : m_mtx(mtx) { pthread_mutex_lock(&m_mtx); }
        ~AutoLock() { pthread_mutex_unlock(&m_mtx); }
    private:
        pthread_mutex_t& m_mtx;
    };
#endif

    IPWhitelist() {
#ifdef _WIN32
        InitializeCriticalSection(&m_Lock);
#else
        pthread_mutex_init(&m_Lock, nullptr);
#endif
    }

    ~IPWhitelist() {
#ifdef _WIN32
        DeleteCriticalSection(&m_Lock);
#else
        pthread_mutex_destroy(&m_Lock);
#endif
    }

    // 禁止拷贝
    IPWhitelist(const IPWhitelist&) = delete;
    IPWhitelist& operator=(const IPWhitelist&) = delete;

    void AddIPInternal(const std::string& ip) {
        std::string trimmed = ip;
        // 去除空格
        while (!trimmed.empty() && trimmed.front() == ' ') trimmed.erase(0, 1);
        while (!trimmed.empty() && trimmed.back() == ' ') trimmed.pop_back();
        if (!trimmed.empty()) {
            m_IPs.insert(trimmed);
        }
    }

    std::set<std::string> m_IPs;
#ifdef _WIN32
    CRITICAL_SECTION m_Lock;
#else
    pthread_mutex_t m_Lock;
#endif
};
