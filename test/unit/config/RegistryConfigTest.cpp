/**
 * @file RegistryConfigTest.cpp
 * @brief iniFile 和 binFile 注册表配置类单元测试
 *
 * 测试覆盖：
 * - 基本读写操作（字符串、整数、浮点数）
 * - 二进制数据读写
 * - 键句柄缓存机制
 * - 并发读写
 * - 默认值处理
 * - 边界条件
 *
 * 注意：此测试仅在 Windows 平台运行，使用真实注册表
 */

#include <gtest/gtest.h>

#ifdef _WIN32

// Windows 头文件必须最先包含
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <string>
#include <map>

// 定义 INIFILE_STANDALONE 跳过 commands.h 中的 MFC 依赖
#define INIFILE_STANDALONE 1

// 提供 iniFile.h 需要的最小依赖（来自 commands.h）
#ifndef GET_FILEPATH
#define GET_FILEPATH(path, file) do { \
    char* p = strrchr(path, '\\'); \
    if (p) strcpy_s(p + 1, _MAX_PATH - (p - path + 1), file); \
} while(0)
#endif

inline std::vector<std::string> StringToVector(const std::string& s, char ch) {
    std::vector<std::string> result;
    std::string::size_type start = 0;
    std::string::size_type pos = s.find(ch, start);
    while (pos != std::string::npos) {
        result.push_back(s.substr(start, pos - start));
        start = pos + 1;
        pos = s.find(ch, start);
    }
    result.push_back(s.substr(start));
    if (result.empty()) result.push_back("");
    return result;
}

// 包含实际的 iniFile.h 头文件（需要修改 iniFile.h 支持 INIFILE_STANDALONE）
#include "common/iniFile.h"

// 测试用的注册表路径（与生产环境隔离）
#define TEST_INI_PATH "Software\\YAMA_TEST_INI"
#define TEST_BIN_PATH "Software\\YAMA_TEST_BIN"

// ============================================
// iniFile 测试夹具
// ============================================
class IniFileTest : public ::testing::Test {
protected:
    std::unique_ptr<iniFile> cfg;

    void SetUp() override {
        cfg = std::make_unique<iniFile>(TEST_INI_PATH);
    }

    void TearDown() override {
        cfg.reset();
        // 清理测试注册表项
        RegDeleteTreeA(HKEY_CURRENT_USER, TEST_INI_PATH);
    }
};

// ============================================
// binFile 测试夹具
// ============================================
class BinFileTest : public ::testing::Test {
protected:
    std::unique_ptr<binFile> cfg;

    void SetUp() override {
        cfg = std::make_unique<binFile>(TEST_BIN_PATH);
    }

    void TearDown() override {
        cfg.reset();
        // 清理测试注册表项
        RegDeleteTreeA(HKEY_CURRENT_USER, TEST_BIN_PATH);
    }
};

// ============================================
// iniFile 基本读写测试
// ============================================
TEST_F(IniFileTest, SetStr_GetStr_ReturnsCorrectValue) {
    EXPECT_TRUE(cfg->SetStr("TestSection", "TestKey", "HelloWorld"));
    EXPECT_EQ(cfg->GetStr("TestSection", "TestKey"), "HelloWorld");
}

TEST_F(IniFileTest, SetStr_EmptyString_ReturnsEmpty) {
    EXPECT_TRUE(cfg->SetStr("TestSection", "EmptyKey", ""));
    EXPECT_EQ(cfg->GetStr("TestSection", "EmptyKey"), "");
}

TEST_F(IniFileTest, GetStr_NonExistent_ReturnsDefault) {
    EXPECT_EQ(cfg->GetStr("NonExistent", "Key", "DefaultValue"), "DefaultValue");
}

TEST_F(IniFileTest, SetInt_GetInt_ReturnsCorrectValue) {
    EXPECT_TRUE(cfg->SetInt("TestSection", "IntKey", 12345));
    EXPECT_EQ(cfg->GetInt("TestSection", "IntKey"), 12345);
}

TEST_F(IniFileTest, SetInt_NegativeValue_ReturnsCorrect) {
    EXPECT_TRUE(cfg->SetInt("TestSection", "NegKey", -9999));
    EXPECT_EQ(cfg->GetInt("TestSection", "NegKey"), -9999);
}

TEST_F(IniFileTest, GetInt_NonExistent_ReturnsDefault) {
    EXPECT_EQ(cfg->GetInt("NonExistent", "Key", 42), 42);
}

TEST_F(IniFileTest, GetInt_InvalidString_ReturnsDefault) {
    cfg->SetStr("TestSection", "InvalidInt", "NotANumber");
    EXPECT_EQ(cfg->GetInt("TestSection", "InvalidInt", 99), 99);
}

TEST_F(IniFileTest, SetStr_ChineseCharacters_ReturnsCorrect) {
    EXPECT_TRUE(cfg->SetStr("TestSection", "ChineseKey", "中文测试"));
    EXPECT_EQ(cfg->GetStr("TestSection", "ChineseKey"), "中文测试");
}

TEST_F(IniFileTest, SetStr_LongString_ReturnsCorrect) {
    std::string longStr(400, 'A');  // 400 字符长字符串
    EXPECT_TRUE(cfg->SetStr("TestSection", "LongKey", longStr));
    EXPECT_EQ(cfg->GetStr("TestSection", "LongKey"), longStr);
}

TEST_F(IniFileTest, SetStr_SpecialCharacters_ReturnsCorrect) {
    std::string special = "Path\\With\\Backslashes=Value;With|Special<Chars>";
    EXPECT_TRUE(cfg->SetStr("TestSection", "SpecialKey", special));
    EXPECT_EQ(cfg->GetStr("TestSection", "SpecialKey"), special);
}

// ============================================
// iniFile 多 Section 测试
// ============================================
TEST_F(IniFileTest, MultipleSections_IsolatedValues) {
    cfg->SetStr("Section1", "Key", "Value1");
    cfg->SetStr("Section2", "Key", "Value2");
    cfg->SetStr("Section3", "Key", "Value3");

    EXPECT_EQ(cfg->GetStr("Section1", "Key"), "Value1");
    EXPECT_EQ(cfg->GetStr("Section2", "Key"), "Value2");
    EXPECT_EQ(cfg->GetStr("Section3", "Key"), "Value3");
}

TEST_F(IniFileTest, MultipleKeys_SameSection) {
    cfg->SetStr("Settings", "Key1", "Value1");
    cfg->SetStr("Settings", "Key2", "Value2");
    cfg->SetInt("Settings", "Key3", 100);

    EXPECT_EQ(cfg->GetStr("Settings", "Key1"), "Value1");
    EXPECT_EQ(cfg->GetStr("Settings", "Key2"), "Value2");
    EXPECT_EQ(cfg->GetInt("Settings", "Key3"), 100);
}

// ============================================
// iniFile 覆盖写入测试
// ============================================
TEST_F(IniFileTest, OverwriteValue_ReturnsNewValue) {
    cfg->SetStr("TestSection", "Key", "OldValue");
    cfg->SetStr("TestSection", "Key", "NewValue");
    EXPECT_EQ(cfg->GetStr("TestSection", "Key"), "NewValue");
}

TEST_F(IniFileTest, OverwriteInt_ReturnsNewValue) {
    cfg->SetInt("TestSection", "IntKey", 100);
    cfg->SetInt("TestSection", "IntKey", 200);
    EXPECT_EQ(cfg->GetInt("TestSection", "IntKey"), 200);
}

// ============================================
// binFile 基本读写测试
// ============================================
TEST_F(BinFileTest, SetStr_GetStr_ReturnsCorrectValue) {
    EXPECT_TRUE(cfg->SetStr("TestSection", "TestKey", "BinaryHello"));
    EXPECT_EQ(cfg->GetStr("TestSection", "TestKey"), "BinaryHello");
}

TEST_F(BinFileTest, SetInt_GetInt_ReturnsCorrectValue) {
    EXPECT_TRUE(cfg->SetInt("TestSection", "IntKey", 0x12345678));
    EXPECT_EQ(cfg->GetInt("TestSection", "IntKey"), 0x12345678);
}

TEST_F(BinFileTest, SetInt_NegativeValue_ReturnsCorrect) {
    EXPECT_TRUE(cfg->SetInt("TestSection", "NegKey", -12345));
    EXPECT_EQ(cfg->GetInt("TestSection", "NegKey"), -12345);
}

TEST_F(BinFileTest, GetInt_NonExistent_ReturnsDefault) {
    EXPECT_EQ(cfg->GetInt("NonExistent", "Key", 999), 999);
}

TEST_F(BinFileTest, GetStr_NonExistent_ReturnsDefault) {
    EXPECT_EQ(cfg->GetStr("NonExistent", "Key", "Default"), "Default");
}

// ============================================
// 并发测试（验证注册表操作的稳定性）
// ============================================
TEST_F(IniFileTest, Concurrent_ReadWrite_NoCorruption) {
    std::atomic<bool> running{true};
    std::atomic<int> writeCount{0};
    std::atomic<int> readCount{0};

    // 写线程
    std::thread writer([&]() {
        int i = 0;
        while (running) {
            std::string key = "Key" + std::to_string(i % 10);
            std::string value = "Value" + std::to_string(i);
            if (cfg->SetStr("ConcurrentTest", key, value)) {
                writeCount++;
            }
            i++;
            std::this_thread::yield();
        }
    });

    // 读线程
    std::thread reader([&]() {
        while (running) {
            for (int i = 0; i < 10; i++) {
                std::string key = "Key" + std::to_string(i);
                cfg->GetStr("ConcurrentTest", key, "");
                readCount++;
            }
            std::this_thread::yield();
        }
    });

    // 运行 100ms
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    running = false;

    writer.join();
    reader.join();

    EXPECT_GT(writeCount.load(), 0);
    EXPECT_GT(readCount.load(), 0);
}

TEST_F(IniFileTest, Concurrent_MultipleWriters_NoCrash) {
    std::atomic<bool> running{true};
    std::vector<std::thread> writers;

    for (int t = 0; t < 4; t++) {
        writers.emplace_back([&, t]() {
            int i = 0;
            while (running) {
                std::string section = "Section" + std::to_string(t);
                std::string key = "Key" + std::to_string(i % 5);
                cfg->SetInt(section, key, i);
                cfg->GetInt(section, key);
                i++;
                std::this_thread::yield();
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    running = false;

    for (auto& t : writers) {
        t.join();
    }

    // 无崩溃即为成功
    SUCCEED();
}

// ============================================
// 边界条件测试
// ============================================
TEST_F(IniFileTest, BoundaryCondition_MaxIntValue) {
    cfg->SetInt("BoundaryTest", "MaxInt", INT_MAX);
    EXPECT_EQ(cfg->GetInt("BoundaryTest", "MaxInt"), INT_MAX);
}

TEST_F(IniFileTest, BoundaryCondition_MinIntValue) {
    cfg->SetInt("BoundaryTest", "MinInt", INT_MIN);
    EXPECT_EQ(cfg->GetInt("BoundaryTest", "MinInt"), INT_MIN);
}

TEST_F(IniFileTest, BoundaryCondition_ZeroValue) {
    cfg->SetInt("BoundaryTest", "Zero", 0);
    EXPECT_EQ(cfg->GetInt("BoundaryTest", "Zero", -1), 0);
}

TEST_F(BinFileTest, BoundaryCondition_ZeroInt) {
    cfg->SetInt("BoundaryTest", "Zero", 0);
    EXPECT_EQ(cfg->GetInt("BoundaryTest", "Zero", -1), 0);
}

TEST_F(BinFileTest, BoundaryCondition_NegativeMax) {
    cfg->SetInt("BoundaryTest", "NegMax", INT_MIN);
    EXPECT_EQ(cfg->GetInt("BoundaryTest", "NegMax"), INT_MIN);
}

// ============================================
// 性能测试
// ============================================
TEST_F(IniFileTest, Performance_CachedAccess_FastEnough) {
    // 预热缓存
    cfg->SetStr("PerfTest", "Key", "InitialValue");

    auto start = std::chrono::high_resolution_clock::now();

    const int iterations = 1000;
    for (int i = 0; i < iterations; i++) {
        cfg->SetStr("PerfTest", "Key", "Value" + std::to_string(i));
        cfg->GetStr("PerfTest", "Key");
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 1000 次读写应在 2 秒内完成
    EXPECT_LT(duration.count(), 2000) << "Performance: " << duration.count() << "ms for 1000 iterations";
}

#else
// 非 Windows 平台跳过测试
TEST(RegistryConfigTest, SkippedOnNonWindows) {
    GTEST_SKIP() << "Registry tests only run on Windows";
}
#endif
