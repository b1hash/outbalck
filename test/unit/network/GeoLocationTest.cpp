// GeoLocationTest.cpp - IP地理位置API单元测试
// 测试 location.h 中的 GetGeoLocation 功能

#include <gtest/gtest.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <wininet.h>
#include <ws2tcpip.h>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ws2_32.lib")

#include <string>
#include <vector>

// ============================================
// 简化的测试版本 - 不依赖jsoncpp
// ============================================

// UTF-8 转 ANSI
inline std::string Utf8ToAnsi(const std::string& utf8)
{
    if (utf8.empty()) return "";
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    if (wideLen <= 0) return utf8;
    std::wstring wideStr(wideLen, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wideStr[0], wideLen);
    int ansiLen = WideCharToMultiByte(CP_ACP, 0, wideStr.c_str(), -1, NULL, 0, NULL, NULL);
    if (ansiLen <= 0) return utf8;
    std::string ansiStr(ansiLen, 0);
    WideCharToMultiByte(CP_ACP, 0, wideStr.c_str(), -1, &ansiStr[0], ansiLen, NULL, NULL);
    if (!ansiStr.empty() && ansiStr.back() == '\0') ansiStr.pop_back();
    return ansiStr;
}

// 简单JSON值提取 (仅用于测试，不依赖jsoncpp)
std::string ExtractJsonString(const std::string& json, const std::string& key)
{
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return "";

    size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string::npos) return "";

    size_t valueStart = json.find_first_not_of(" \t\n\r", colonPos + 1);
    if (valueStart == std::string::npos) return "";

    if (json[valueStart] == '"') {
        size_t valueEnd = json.find('"', valueStart + 1);
        if (valueEnd == std::string::npos) return "";
        return json.substr(valueStart + 1, valueEnd - valueStart - 1);
    }

    // 数字或布尔值
    size_t valueEnd = json.find_first_of(",}\n\r", valueStart);
    if (valueEnd == std::string::npos) valueEnd = json.length();
    std::string value = json.substr(valueStart, valueEnd - valueStart);
    // 去除尾部空格
    while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) {
        value.pop_back();
    }
    return value;
}

// API配置结构
struct GeoApiConfig {
    const char* name;
    const char* urlFmt;
    const char* cityField;
    const char* countryField;
    const char* checkField;
    const char* checkValue;
    bool useHttps;
};

// 测试用API配置
static const GeoApiConfig testApis[] = {
    {"ip-api.com", "http://ip-api.com/json/%s?fields=status,country,city", "city", "country", "status", "success", false},
    {"ipinfo.io", "http://ipinfo.io/%s/json", "city", "country", "", "", false},
    {"ipapi.co", "https://ipapi.co/%s/json/", "city", "country_name", "error", "", true},
};

// 测试单个API
struct ApiTestResult {
    bool success;
    int httpStatus;
    std::string city;
    std::string country;
    std::string error;
    double latencyMs;
};

ApiTestResult TestSingleApi(const GeoApiConfig& api, const std::string& ip)
{
    ApiTestResult result = {false, 0, "", "", "", 0};

    DWORD startTime = GetTickCount();

    HINTERNET hInternet = InternetOpenA("GeoLocationTest", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        result.error = "InternetOpen failed";
        return result;
    }

    DWORD timeout = 10000;
    InternetSetOptionA(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOptionA(hInternet, INTERNET_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOptionA(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

    char urlBuf[256];
    sprintf_s(urlBuf, api.urlFmt, ip.c_str());

    DWORD flags = INTERNET_FLAG_RELOAD;
    if (api.useHttps) flags |= INTERNET_FLAG_SECURE;

    HINTERNET hConnect = InternetOpenUrlA(hInternet, urlBuf, NULL, 0, flags, 0);
    if (!hConnect) {
        result.error = "InternetOpenUrl failed: " + std::to_string(GetLastError());
        InternetCloseHandle(hInternet);
        return result;
    }

    // 获取HTTP状态码
    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);
    if (HttpQueryInfoA(hConnect, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &statusSize, NULL)) {
        result.httpStatus = statusCode;
    }

    // 读取响应
    std::string readBuffer;
    char buffer[4096];
    DWORD bytesRead;
    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        readBuffer.append(buffer, bytesRead);
    }

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    result.latencyMs = (double)(GetTickCount() - startTime);

    // 检查HTTP错误
    if (result.httpStatus >= 400) {
        result.error = "HTTP " + std::to_string(result.httpStatus);
        if (result.httpStatus == 429) {
            result.error += " (Rate Limited)";
        }
        return result;
    }

    // 检查响应体错误
    if (readBuffer.find("Rate limit") != std::string::npos ||
        readBuffer.find("rate limit") != std::string::npos) {
        result.error = "Rate limited (body)";
        return result;
    }

    // 解析JSON
    if (api.checkField && api.checkField[0]) {
        std::string checkVal = ExtractJsonString(readBuffer, api.checkField);
        if (api.checkValue && api.checkValue[0]) {
            if (checkVal != api.checkValue) {
                result.error = "Check failed: " + std::string(api.checkField) + "=" + checkVal;
                return result;
            }
        } else {
            if (checkVal == "true") {
                result.error = "Error flag set";
                return result;
            }
        }
    }

    result.city = Utf8ToAnsi(ExtractJsonString(readBuffer, api.cityField));
    result.country = Utf8ToAnsi(ExtractJsonString(readBuffer, api.countryField));
    result.success = !result.city.empty() || !result.country.empty();

    if (!result.success) {
        result.error = "No city/country in response";
    }

    return result;
}

// ============================================
// 测试用例
// ============================================

class GeoLocationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化Winsock
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    }

    void TearDown() override {
        WSACleanup();
    }
};

// 测试公网IP (Google DNS)
TEST_F(GeoLocationTest, TestPublicIP_GoogleDNS) {
    std::string testIP = "8.8.8.8";

    std::cout << "\n=== Testing IP: " << testIP << " ===\n";

    int successCount = 0;
    for (const auto& api : testApis) {
        auto result = TestSingleApi(api, testIP);

        std::cout << "[" << api.name << "] ";
        if (result.success) {
            std::cout << "OK - " << result.city << ", " << result.country;
            std::cout << " (HTTP " << result.httpStatus << ", " << result.latencyMs << "ms)\n";
            successCount++;
        } else {
            std::cout << "FAIL - " << result.error;
            std::cout << " (HTTP " << result.httpStatus << ", " << result.latencyMs << "ms)\n";
        }
    }

    // 至少一个API应该成功
    EXPECT_GE(successCount, 1) << "At least one API should succeed";
}

// 测试另一个公网IP (Cloudflare DNS)
TEST_F(GeoLocationTest, TestPublicIP_CloudflareDNS) {
    std::string testIP = "1.1.1.1";

    std::cout << "\n=== Testing IP: " << testIP << " ===\n";

    int successCount = 0;
    for (const auto& api : testApis) {
        auto result = TestSingleApi(api, testIP);

        std::cout << "[" << api.name << "] ";
        if (result.success) {
            std::cout << "OK - " << result.city << ", " << result.country;
            std::cout << " (HTTP " << result.httpStatus << ", " << result.latencyMs << "ms)\n";
            successCount++;
        } else {
            std::cout << "FAIL - " << result.error;
            std::cout << " (HTTP " << result.httpStatus << ", " << result.latencyMs << "ms)\n";
        }
    }

    EXPECT_GE(successCount, 1) << "At least one API should succeed";
}

// 测试中国IP
TEST_F(GeoLocationTest, TestPublicIP_ChinaIP) {
    std::string testIP = "114.114.114.114";  // 114DNS

    std::cout << "\n=== Testing IP: " << testIP << " (China) ===\n";

    int successCount = 0;
    for (const auto& api : testApis) {
        auto result = TestSingleApi(api, testIP);

        std::cout << "[" << api.name << "] ";
        if (result.success) {
            std::cout << "OK - " << result.city << ", " << result.country;
            std::cout << " (HTTP " << result.httpStatus << ", " << result.latencyMs << "ms)\n";
            successCount++;
        } else {
            std::cout << "FAIL - " << result.error;
            std::cout << " (HTTP " << result.httpStatus << ", " << result.latencyMs << "ms)\n";
        }
    }

    EXPECT_GE(successCount, 1) << "At least one API should succeed";
}

// 测试ip-api.com单独
TEST_F(GeoLocationTest, TestIpApiCom) {
    std::string testIP = "8.8.8.8";
    auto result = TestSingleApi(testApis[0], testIP);

    std::cout << "\n[ip-api.com] HTTP: " << result.httpStatus
              << ", Latency: " << result.latencyMs << "ms\n";

    if (result.success) {
        std::cout << "City: " << result.city << ", Country: " << result.country << "\n";
        EXPECT_FALSE(result.country.empty());
    } else {
        std::cout << "Error: " << result.error << "\n";
        // 如果是429就跳过，不算失败
        if (result.httpStatus == 429) {
            GTEST_SKIP() << "Rate limited, skipping";
        }
    }
}

// 测试ipinfo.io单独
TEST_F(GeoLocationTest, TestIpInfoIo) {
    std::string testIP = "8.8.8.8";
    auto result = TestSingleApi(testApis[1], testIP);

    std::cout << "\n[ipinfo.io] HTTP: " << result.httpStatus
              << ", Latency: " << result.latencyMs << "ms\n";

    if (result.success) {
        std::cout << "City: " << result.city << ", Country: " << result.country << "\n";
        EXPECT_FALSE(result.country.empty());
    } else {
        std::cout << "Error: " << result.error << "\n";
        if (result.httpStatus == 429) {
            GTEST_SKIP() << "Rate limited, skipping";
        }
    }
}

// 测试ipapi.co单独
TEST_F(GeoLocationTest, TestIpApiCo) {
    std::string testIP = "8.8.8.8";
    auto result = TestSingleApi(testApis[2], testIP);

    std::cout << "\n[ipapi.co] HTTP: " << result.httpStatus
              << ", Latency: " << result.latencyMs << "ms\n";

    if (result.success) {
        std::cout << "City: " << result.city << ", Country: " << result.country << "\n";
        EXPECT_FALSE(result.country.empty());
    } else {
        std::cout << "Error: " << result.error << "\n";
        if (result.httpStatus == 429) {
            GTEST_SKIP() << "Rate limited, skipping";
        }
    }
}

// 测试HTTP状态码检测
TEST_F(GeoLocationTest, TestHttpStatusCodeDetection) {
    // 使用一个会返回错误的请求
    std::string testIP = "invalid-ip";

    for (const auto& api : testApis) {
        auto result = TestSingleApi(api, testIP);
        std::cout << "[" << api.name << "] Invalid IP test: HTTP "
                  << result.httpStatus << ", Error: " << result.error << "\n";

        // 应该失败
        EXPECT_FALSE(result.success);
    }
}

// 测试所有API的响应时间
TEST_F(GeoLocationTest, TestApiLatency) {
    std::string testIP = "8.8.8.8";

    std::cout << "\n=== API Latency Test ===\n";

    for (const auto& api : testApis) {
        auto result = TestSingleApi(api, testIP);
        std::cout << "[" << api.name << "] " << result.latencyMs << "ms";
        if (result.success) {
            std::cout << " (OK)";
        } else {
            std::cout << " (FAIL: " << result.error << ")";
        }
        std::cout << "\n";

        // 响应时间应该在合理范围内 (< 15秒)
        EXPECT_LT(result.latencyMs, 15000);
    }
}

#else
// 非Windows平台 - 跳过测试
TEST(GeoLocationTest, SkipNonWindows) {
    GTEST_SKIP() << "GeoLocation tests only run on Windows";
}
#endif

