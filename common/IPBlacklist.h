// IPBlacklist.h - IP 黑名单管理 (单例)
// 用于拒绝特定 IP 的所有请求

#pragma once
#include <string>
#include <set>
#include <map>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

class IPBlacklist {
public:
    static IPBlacklist& getInstance() {
        static IPBlacklist instance;
        return instance;
    }

    // 从配置字符串加载黑名单 (格式: "192.168.1.1;10.0.0.1")
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

    // 检查 IP 是否在黑名单中
    bool IsBlacklisted(const std::string& ip) {
        // 本地地址永不加入黑名单
        if (ip == "127.0.0.1" || ip == "::1") {
            return false;
        }

        AutoLock lock(m_Lock);
        return m_IPs.find(ip) != m_IPs.end();
    }

    // 添加 IP 到黑名单
    void AddIP(const std::string& ip) {
        if (ip == "127.0.0.1" || ip == "::1") {
            return;  // 本地地址不能加入黑名单
        }
        AutoLock lock(m_Lock);
        AddIPInternal(ip);
    }

    // 从黑名单移除 IP
    void RemoveIP(const std::string& ip) {
        AutoLock lock(m_Lock);
        m_IPs.erase(ip);
    }

    // 获取黑名单数量
    size_t Count() {
        AutoLock lock(m_Lock);
        return m_IPs.size();
    }

    // 获取所有黑名单 IP（用于显示）
    std::set<std::string> GetAll() {
        AutoLock lock(m_Lock);
        return m_IPs;
    }

    // 清空黑名单
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

    // 检查是否应该记录日志（防刷频，同一 IP 每 300 秒最多记录一次）
    bool ShouldLog(const std::string& ip) {
        AutoLock lock(m_Lock);
        time_t now = time(nullptr);

        // 定期清理过期的日志时间记录 (每 100 次检查一次，或条目超过 1000)
        if (++m_CleanupCounter >= 100 || m_LastLogTime.size() > 1000) {
            m_CleanupCounter = 0;
            for (auto it = m_LastLogTime.begin(); it != m_LastLogTime.end(); ) {
                if (now - it->second >= 300) {  // 5分钟未活动则清理
                    it = m_LastLogTime.erase(it);
                } else {
                    ++it;
                }
            }
        }

        auto it = m_LastLogTime.find(ip);
        if (it == m_LastLogTime.end() || (now - it->second) >= 300) {
            m_LastLogTime[ip] = now;
            return true;
        }
        return false;
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

    IPBlacklist() {
#ifdef _WIN32
        InitializeCriticalSection(&m_Lock);
#else
        pthread_mutex_init(&m_Lock, nullptr);
#endif
    }

    ~IPBlacklist() {
#ifdef _WIN32
        DeleteCriticalSection(&m_Lock);
#else
        pthread_mutex_destroy(&m_Lock);
#endif
    }

    // 禁止拷贝
    IPBlacklist(const IPBlacklist&) = delete;
    IPBlacklist& operator=(const IPBlacklist&) = delete;

    void AddIPInternal(const std::string& ip) {
        std::string trimmed = ip;
        // 去除空格
        while (!trimmed.empty() && trimmed.front() == ' ') trimmed.erase(0, 1);
        while (!trimmed.empty() && trimmed.back() == ' ') trimmed.pop_back();
        if (!trimmed.empty() && trimmed != "127.0.0.1" && trimmed != "::1") {
            m_IPs.insert(trimmed);
        }
    }

    std::set<std::string> m_IPs;
    std::map<std::string, time_t> m_LastLogTime;  // 防刷频：记录每个 IP 最后日志时间
    int m_CleanupCounter = 0;  // 清理计数器
#ifdef _WIN32
    CRITICAL_SECTION m_Lock;
#else
    pthread_mutex_t m_Lock;
#endif
};
