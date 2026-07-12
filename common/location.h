#pragma once
#include <string>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <WinInet.h>
#include "logger.h"
#include "jsoncpp/json.h"

#ifndef _WIN64
#ifdef _DEBUG
#pragma comment(lib, "jsoncpp/jsoncppd.lib")
#else
#pragma comment(lib, "jsoncpp/jsoncpp.lib")
#endif
#else
#ifdef _DEBUG
#pragma comment(lib, "jsoncpp/jsoncpp_x64d.lib")
#else
#pragma comment(lib, "jsoncpp/jsoncpp_x64.lib")
#endif
#endif

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "Iphlpapi.lib")

// UTF-8 转 ANSI（当前系统代码页）
inline std::string Utf8ToAnsi(const std::string& utf8)
{
    if (utf8.empty()) return "";

    // UTF-8 -> UTF-16
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    if (wideLen <= 0) return utf8;

    std::wstring wideStr(wideLen, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wideStr[0], wideLen);

    // UTF-16 -> ANSI
    int ansiLen = WideCharToMultiByte(CP_ACP, 0, wideStr.c_str(), -1, NULL, 0, NULL, NULL);
    if (ansiLen <= 0) return utf8;

    std::string ansiStr(ansiLen, 0);
    WideCharToMultiByte(CP_ACP, 0, wideStr.c_str(), -1, &ansiStr[0], ansiLen, NULL, NULL);

    // 去掉末尾的 null 字符
    if (!ansiStr.empty() && ansiStr.back() == '\0') {
        ansiStr.pop_back();
    }

    return ansiStr;
}

// 检测字符串是否可能是 UTF-8 编码（包含多字节序列）
inline bool IsLikelyUtf8(const std::string& str)
{
    for (size_t i = 0; i < str.size(); i++) {
        unsigned char c = str[i];
        if (c >= 0x80) {
            // 2字节序列: 110xxxxx 10xxxxxx
            if ((c & 0xE0) == 0xC0 && i + 1 < str.size()) {
                if ((str[i+1] & 0xC0) == 0x80) return true;
            }
            // 3字节序列: 1110xxxx 10xxxxxx 10xxxxxx
            else if ((c & 0xF0) == 0xE0 && i + 2 < str.size()) {
                if ((str[i+1] & 0xC0) == 0x80 && (str[i+2] & 0xC0) == 0x80) return true;
            }
            // 4字节序列: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            else if ((c & 0xF8) == 0xF0 && i + 3 < str.size()) {
                if ((str[i+1] & 0xC0) == 0x80 && (str[i+2] & 0xC0) == 0x80 && (str[i+3] & 0xC0) == 0x80) return true;
            }
        }
    }
    return false;
}

// 安全转换：检测到 UTF-8 才转换，否则原样返回
inline std::string SafeUtf8ToAnsi(const std::string& str)
{
    if (str.empty()) return str;
    return IsLikelyUtf8(str) ? Utf8ToAnsi(str) : str;
}

inline void splitIpPort(const std::string& input, std::string& ip, std::string& port)
{
    size_t pos = input.find(':');
    if (pos != std::string::npos) {
        ip = input.substr(0, pos);
        port = input.substr(pos + 1);
    } else {
        ip = input;
        port = "";
    }
}

/**
 * IPConverter: IP 操作类，用于获取公网IP，获取IP对应的地理位置等.
 * 目前是通过调用公开的互联网API完成，假如该查询网站不可访问则需要重新适配.
 */
class IPConverter
{
public:
    virtual ~IPConverter() {}

    virtual std::string IPtoAddress(const std::string& ip)
    {
        return "implement me";
    }

    /**
     * 判断给定的 IP 地址是否是局域网（内网）IP
     * @param ipAddress IP 地址字符串（如 "192.168.1.1"）
     * @return 如果是局域网 IP，返回 true; 否则返回 false
     */
    bool IsPrivateIP(const std::string& ipAddress)
    {
        // 将 IP 地址字符串转换为二进制格式
        in_addr addr;
        if (inet_pton(AF_INET, ipAddress.c_str(), &addr) != 1) {
            Mprintf("Invalid IP address: %s\n", ipAddress.c_str());
            return false;
        }

        // 将二进制 IP 地址转换为无符号整数
        unsigned long ip = ntohl(addr.s_addr);

        // 检查 IP 地址是否在局域网范围内
        if ((ip >= 0x0A000000 && ip <= 0x0AFFFFFF) || // 10.0.0.0/8
            (ip >= 0xAC100000 && ip <= 0xAC1FFFFF) || // 172.16.0.0/12
            (ip >= 0xC0A80000 && ip <= 0xC0A8FFFF) || // 192.168.0.0/16
            (ip >= 0x7F000000 && ip <= 0x7FFFFFFF)) { // 127.0.0.0/8
            return true;
        }

        return false;
    }

    /**
     * 判断给定的 IP 地址是否是公网 IP
     */
    bool IsPublicIP(const std::string& ipAddress)
    {
        in_addr addr;
        if (inet_pton(AF_INET, ipAddress.c_str(), &addr) != 1) return false;
        unsigned long h = ntohl(addr.s_addr);
        if ((h & 0xFF000000) == 0x0A000000) return false;  // 10.0.0.0/8
        if ((h & 0xFFF00000) == 0xAC100000) return false;  // 172.16.0.0/12
        if ((h & 0xFFFF0000) == 0xC0A80000) return false;  // 192.168.0.0/16
        if ((h & 0xFF000000) == 0x7F000000) return false;  // 127.0.0.0/8
        if ((h & 0xFF000000) == 0x00000000) return false;  // 0.0.0.0/8
        if ((h & 0xFFFF0000) == 0xA9FE0000) return false;  // 169.254.0.0/16
        if ((h & 0xFFC00000) == 0x64400000) return false;  // 100.64.0.0/10
        if ((h & 0xF0000000) == 0xE0000000) return false;  // 224.0.0.0/4
        if ((h & 0xF0000000) == 0xF0000000) return false;  // 240.0.0.0/4
        return true;
    }

    /**
     * 获取本机网卡的公网 IP 和私有 IP
     */
    void GetLocalIPs(std::string& publicIP, std::string& privateIP)
    {
        publicIP.clear();
        privateIP.clear();
        ULONG outBufLen = 15000;
        IP_ADAPTER_ADDRESSES* pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
        if (GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, nullptr, pAddresses, &outBufLen) == ERROR_BUFFER_OVERFLOW) {
            free(pAddresses);
            pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
        }
        if (GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, nullptr, pAddresses, &outBufLen) == NO_ERROR) {
            for (IP_ADAPTER_ADDRESSES* pCurr = pAddresses; pCurr; pCurr = pCurr->Next) {
                if (pCurr->OperStatus != IfOperStatusUp) continue;
                if (pCurr->IfType == IF_TYPE_SOFTWARE_LOOPBACK) continue;
                for (IP_ADAPTER_UNICAST_ADDRESS* pUnicast = pCurr->FirstUnicastAddress; pUnicast; pUnicast = pUnicast->Next) {
                    if (pUnicast->Address.lpSockaddr->sa_family != AF_INET) continue;
                    char addrBuf[INET_ADDRSTRLEN] = { 0 };
                    sockaddr_in* sa_in = (sockaddr_in*)pUnicast->Address.lpSockaddr;
                    inet_ntop(AF_INET, &sa_in->sin_addr, addrBuf, sizeof(addrBuf));
                    std::string ip = addrBuf;
                    if (ip.substr(0, 4) == "127.") continue;
                    if (IsPublicIP(ip)) { if (publicIP.empty()) publicIP = ip; }
                    else { if (privateIP.empty()) privateIP = ip; }
                    if (!publicIP.empty() && !privateIP.empty()) break;
                }
                if (!publicIP.empty() && !privateIP.empty()) break;
            }
        }
        free(pAddresses);
    }

    // 获取本机地理位置
    std::string GetLocalLocation()
    {
        return GetGeoLocation(getPublicIP());
    }

    // 获取 IP 地址地理位置 (多API备用)
    virtual std::string GetGeoLocation(const std::string& IP)
    {
        if (IP.empty()) return "";

        std::string ip = IP;
        if (isLocalIP(ip)) {
            ip = getPublicIP();
            if (ip.empty())
                return "";
        }

        // 初始化 WinINet
        HINTERNET hInternet = InternetOpen("IP Geolocation", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (hInternet == NULL) {
            Mprintf("InternetOpen failed! %d\n", GetLastError());
            return "";
        }

        // 设置超时
        DWORD timeout = 5000;
        InternetSetOptionA(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
        InternetSetOptionA(hInternet, INTERNET_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
        InternetSetOptionA(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

        // API配置: {url格式, 城市字段, 国家字段, 成功条件字段, 成功条件值, 是否HTTPS}
        struct GeoApiConfig {
            const char* urlFmt;      // URL格式 (%s = IP)
            const char* cityField;   // 城市字段名
            const char* countryField;// 国家字段名
            const char* checkField;  // 校验字段 (空=不校验)
            const char* checkValue;  // 校验值 (空=检查非error)
            bool useHttps;
        };

        static const GeoApiConfig apis[] = {
            // ip-api.com: 45次/分钟
            {"http://ip-api.com/json/%s?fields=status,country,city", "city", "country", "status", "success", false},
            // ipinfo.io: 1000次/月
            {"http://ipinfo.io/%s/json", "city", "country", "", "", false},
            // ipapi.co: 1000次/天
            {"https://ipapi.co/%s/json/", "city", "country_name", "error", "", true},
        };

        std::string location;
        for (const auto& api : apis) {
            location = TryGeoApi(hInternet, ip, api.urlFmt, api.cityField, api.countryField,
                                 api.checkField, api.checkValue, api.useHttps);
            if (!location.empty()) break;
        }

        InternetCloseHandle(hInternet);

        if (location.empty() && IsPrivateIP(ip)) {
            return "Local Area Network";
        }
        return location;
    }

private:
    // 通用API查询函数
    std::string TryGeoApi(HINTERNET hInternet, const std::string& ip,
                          const char* urlFmt, const char* cityField, const char* countryField,
                          const char* checkField, const char* checkValue, bool useHttps)
    {
        char urlBuf[256];
        sprintf_s(urlBuf, urlFmt, ip.c_str());

        DWORD flags = INTERNET_FLAG_RELOAD;
        if (useHttps) flags |= INTERNET_FLAG_SECURE;

        HINTERNET hConnect = InternetOpenUrlA(hInternet, urlBuf, NULL, 0, flags, 0);
        if (!hConnect) return "";

        // 检查HTTP状态码
        DWORD statusCode = 0;
        DWORD statusSize = sizeof(statusCode);
        if (HttpQueryInfo(hConnect, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &statusSize, NULL)) {
            if (statusCode == 429 || statusCode >= 400) {
                InternetCloseHandle(hConnect);
                return "";  // 429或其他错误，尝试下一个API
            }
        }

        std::string readBuffer;
        char buffer[4096];
        DWORD bytesRead;
        while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            readBuffer.append(buffer, bytesRead);
        }
        InternetCloseHandle(hConnect);

        // 备用: 检查响应体中的错误信息
        if (readBuffer.find("Rate limit") != std::string::npos ||
            readBuffer.find("rate limit") != std::string::npos) {
            return "";
        }

        Json::Value json;
        Json::Reader reader;
        if (!reader.parse(readBuffer, json)) return "";

        // 校验条件
        if (checkField && checkField[0]) {
            if (checkValue && checkValue[0]) {
                // 检查字段==值
                if (json[checkField].asString() != checkValue) return "";
            } else {
                // 检查字段不为true (error字段)
                if (json[checkField].asBool()) return "";
            }
        }

        std::string city = Utf8ToAnsi(json[cityField].asString());
        std::string country = Utf8ToAnsi(json[countryField].asString());

        if (!city.empty() && !country.empty()) return city + ", " + country;
        if (!city.empty()) return city;
        if (!country.empty()) return country;
        return "";
    }

public:

    bool isLoopbackAddress(const std::string& ip)
    {
        return (ip == "127.0.0.1" || ip == "::1");
    }

    bool isLocalIP(const std::string& ip)
    {
        if (isLoopbackAddress(ip)) return true; // 先检查回环地址

        ULONG outBufLen = 15000;
        IP_ADAPTER_ADDRESSES* pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
        if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, pAddresses, &outBufLen) == ERROR_BUFFER_OVERFLOW) {
            free(pAddresses);
            pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
        }

        if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, pAddresses, &outBufLen) == NO_ERROR) {
            for (IP_ADAPTER_ADDRESSES* pCurrAddresses = pAddresses; pCurrAddresses; pCurrAddresses = pCurrAddresses->Next) {
                for (IP_ADAPTER_UNICAST_ADDRESS* pUnicast = pCurrAddresses->FirstUnicastAddress; pUnicast; pUnicast = pUnicast->Next) {
                    char addressBuffer[INET6_ADDRSTRLEN] = { 0 };
                    getnameinfo(pUnicast->Address.lpSockaddr, pUnicast->Address.iSockaddrLength, addressBuffer, sizeof(addressBuffer), nullptr, 0, NI_NUMERICHOST);

                    if (ip == addressBuffer) {
                        free(pAddresses);
                        return true;
                    }
                }
            }
        }

        free(pAddresses);
        return false;
    }

    // 获取公网IP, 获取失败返回空
    std::string getPublicIP()
    {
        clock_t t = clock();

        // 多个候选查询源
        static const std::vector<std::string> urls = {
            "https://checkip.amazonaws.com",  // 全球最稳
            "https://api.ipify.org",          // 主流高可用
            "https://ipinfo.io/ip",           // 备用方案
            "https://icanhazip.com",          // 轻量快速
            "https://ifconfig.me/ip"          // 末位兜底
        };

        // 打开 WinINet 会话
        HINTERNET hInternet = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) {
            Mprintf("InternetOpen failed. cost %d ms.\n", clock() - t);
            return "";
        }

        // 设置超时 (毫秒)
        DWORD timeout = 3000; // 3 秒
        InternetSetOptionA(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
        InternetSetOptionA(hInternet, INTERNET_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
        InternetSetOptionA(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

        std::string result;
        char buffer[2048];
        DWORD bytesRead = 0;

        // 轮询不同 IP 查询源
        for (const auto& url : urls) {
            HINTERNET hConnect = InternetOpenUrlA(
                                     hInternet, url.c_str(), NULL, 0,
                                     INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_CACHE_WRITE,
                                     0
                                 );

            if (!hConnect) {
                continue; // 当前源失败，尝试下一个
            }

            memset(buffer, 0, sizeof(buffer));
            if (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
                result.assign(buffer, bytesRead);

                // 去除换行符和空格
                while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' '))
                    result.pop_back();

                InternetCloseHandle(hConnect);
                break; // 成功获取，停止尝试
            }

            InternetCloseHandle(hConnect);
        }

        InternetCloseHandle(hInternet);

        Mprintf("getPublicIP %s cost %d ms.\n", result.empty() ? "failed" : "succeed", clock() - t);

        return result;
    }
};

enum IPDatabaseType {
    NoneDB = 0,
    QQWry = 1,
    Ip2Region = 2,
};

IPConverter* LoadFileQQWry(const char* datPath);

IPConverter* LoadFileIp2Region(const char* xdbPath);
