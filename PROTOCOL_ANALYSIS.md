# 通讯协议分析文档

本文档详细说明了项目中各种通讯协议的加解密方法、压缩机制、处理顺序以及客户端管理流程。

---

## 目录

1. [通讯协议类型](#1-通讯协议类型)
2. [加密解密方法](#2-加密解密方法)
3. [压缩方法](#3-压缩方法)
4. [数据包处理顺序](#4-数据包处理顺序)
5. [客户端加入列表流程](#5-客户端加入列表流程)
6. [代码文件索引](#6-代码文件索引)

---

## 1. 通讯协议类型

项目支持以下四种通讯协议类型：

| 协议类型 | 标识符 | 标志字符串 | 标志长度 | 头部长度 |
|---------|--------|-----------|---------|---------|
| **HELL** | FLAG_HELL (4) | "HELL" | 8字节 | 16字节 |
| **Shine** | FLAG_SHINE (1) | "Shine" | 5字节 | 13字节 |
| **FUCK** | FLAG_FUCK (2) | "<<FUCK>>" | 11字节 | 19字节 |
| **Hello** | FLAG_HELLO (3) | "Hello?" | 8字节 | 16字节 |

定义位置: `common/header.h`

```cpp
enum FlagType {
    FLAG_WINOS = -1,
    FLAG_UNKNOWN = 0,
    FLAG_SHINE = 1,
    FLAG_FUCK = 2,
    FLAG_HELLO = 3,
    FLAG_HELL = 4,
};
```

---

## 2. 加密解密方法

### 2.1 协议头加密 (Header Encryption)

协议头有8种加密版本 (V0-V7)，用于保护数据包的标识符部分。

| 版本 | 名称 | 算法描述 | 是否对称 |
|-----|------|---------|---------|
| **V0 (None)** | default_encrypt/decrypt | 无加密操作 | ✓ |
| **V1 (Default)** | encrypt/decrypt | 动态密钥XOR + 位置操作 (加/减/XOR/取反) | ✗ |
| **V2** | encrypt_v1/decrypt_v1 | 交替加减操作 | ✗ |
| **V3** | encrypt_v2/decrypt_v2 | XOR + 位旋转 | ✗ |
| **V4** | encrypt_v3/decrypt_v3 | 动态密钥 + 索引变换 | ✗ |
| **V5** | encrypt_v4/decrypt_v4 | 伪随机XOR (LCG生成器) | ✓ |
| **V6** | encrypt_v5/decrypt_v5 | 动态密钥派生 + 位移 | ✗ |
| **V7** | encrypt_v6/decrypt_v6 | 伪随机流混淆 | ✓ |

**代码文件**: `common/header.h`, `common/encfuncs.h`

#### V1 (默认加密) 算法示例:

```cpp
inline void encrypt(unsigned char* data, size_t length, unsigned char key)
{
    if (key == 0) return;
    for (size_t i = 0; i < length; ++i) {
        unsigned char k = static_cast<unsigned char>(key ^ (i * 31));  // 基于位置动态派生密钥
        int value = static_cast<int>(data[i]);
        switch (i % 4) {
        case 0: value += k; break;          // 加法
        case 1: value = value ^ k; break;    // XOR
        case 2: value -= k; break;           // 减法
        case 3: value = ~(value ^ k); break; // XOR + 取反
        }
        data[i] = static_cast<unsigned char>(value & 0xFF);
    }
}
```

### 2.2 数据体加密 (Body Encryption / Encoder)

数据体加密在压缩前/后进行，有以下几种编码器：

| 编码器 | 适用协议 | 算法描述 | 需要加密 |
|-------|---------|---------|---------|
| **Encoder (默认)** | 所有 | 无操作 | ✗ |
| **XOREncoder** | 通用 | 多密钥XOR链式加密 | ✓ |
| **XOREncoder16** | HELL/HELLO | 伪随机交换 + XOR (可选AES-CBC) | ✓ |
| **WinOsEncoder** | WinOS | 自定义XOR变体 | ✓ |

**代码文件**: `common/encrypt.h`

#### XOREncoder16 算法:

```cpp
void encrypt_internal(unsigned char* data, int len, unsigned char k1, unsigned char k2) const
{
    uint16_t key = ((k1 << 8) | k2);
    
    // 第1步: XOR编码
    for (int i = 0; i < len; ++i) {
        data[i] ^= (k1 + i * 13) ^ (k2 ^ (i << 1));
    }

    // 第2步: 两轮伪随机交换
    for (int round = 0; round < 2; ++round) {
        for (int i = 0; i < len; ++i) {
            int j = pseudo_random(key, i + round * 100) % len;
            std::swap(data[i], data[j]);
        }
    }
}
```

#### 可选 AES-CBC 加密:

当 `param[7] == 1` 时，使用AES-CBC加密代替XOREncoder16：

```cpp
static const unsigned char aes_key[16] = {
    0x5A, 0xC3, 0x17, 0xF0, 0x89, 0xB6, 0x4E, 0x7D,
    0x1A, 0x22, 0x9F, 0xC8, 0xD3, 0xE6, 0x73, 0xB1
};
// IV 来自 param + 8
```

### 2.3 数据包伪装 (Packet Masking)

用于将数据包伪装成HTTP请求，以规避网络检测。

| 伪装类型 | 描述 |
|---------|-----|
| **MaskTypeNone** | 无伪装 |
| **MaskTypeHTTP** | 伪装为HTTP POST请求 |

**代码文件**: `common/mask.h`

HTTP伪装格式：
```
POST /[random_path]/[cmd] HTTP/1.1
Host: example.com
User-Agent: Mozilla/5.0 (Windows NT...)
Content-Type: application/octet-stream
Content-Length: [size]
Connection: keep-alive

[原始数据]
```

---

## 3. 压缩方法

### 3.1 支持的压缩算法

| 压缩方法 | 枚举值 | 描述 | 使用场景 |
|---------|-------|-----|---------|
| **ZSTD** | COMPRESS_ZSTD (0) | Zstandard压缩 (默认) | 主要压缩算法 |
| **ZLIB** | COMPRESS_ZLIB (-1) | zlib压缩 | 兼容旧版本 |
| **NONE** | COMPRESS_NONE (1) | 不压缩 | 小数据或已压缩数据 |

**代码文件**: `client/IOCPClient.cpp`, `server/2015Remote/IOCPServer.cpp`

### 3.2 ZSTD 配置参数

```cpp
ZSTD_CCtx_setParameter(m_Cctx, ZSTD_c_nbWorkers, 0);      // 单线程 (可配置多线程)
ZSTD_CCtx_setParameter(m_Cctx, ZSTD_c_compressionLevel, ZSTD_CLEVEL_DEFAULT);
ZSTD_CCtx_setParameter(m_Cctx, ZSTD_c_hashLog, 15);
ZSTD_CCtx_setParameter(m_Cctx, ZSTD_c_chainLog, 16);
ZSTD_CCtx_setParameter(m_Cctx, ZSTD_c_searchLog, 1);
ZSTD_CCtx_setParameter(m_Cctx, ZSTD_c_windowLog, 19);
```

### 3.3 多线程压缩

当数据量超过1MB时，可启用多线程压缩：

```cpp
void IOCPClient::SetMultiThreadCompress(int threadNum)
{
    if (threadNum > 1) {
        ZSTD_CCtx_setParameter(m_Cctx, ZSTD_c_nbWorkers, threadNum);
    }
}
```

---

## 4. 数据包处理顺序

### 4.1 发送流程 (客户端 → 服务端)

```
原始数据 ──► 数据体编码 ──► 压缩 ──► 头部编码 ──► 构建数据包 ──► 伪装 ──► 分块发送
         (Encode)    (Compress)  (Header Enc)              (Mask)   (Split)
```

**详细步骤**:

1. **数据体预处理 (Pre-Encode)**: 可选，对原始数据进行预处理
2. **压缩 (Compress)**: 使用ZSTD/ZLIB压缩数据
3. **头部加密 (Header Encrypt)**: 生成并加密协议头 (包含密钥)
4. **数据体加密 (Body Encrypt)**: XOREncoder16对压缩后数据加密
5. **构建数据包**: 头部 + 压缩长度 + 原始长度 + 压缩数据
6. **伪装 (Mask)**: 可选，包装为HTTP请求
7. **分块发送**: 大数据分块发送 (默认256KB/块)

**代码实现** (`client/IOCPClient.cpp` - `OnServerSending` 函数):

```cpp
BOOL IOCPClient::OnServerSending(const char* szBuffer, ULONG ulOriginalLength, PkgMask* mask)
{
    // 1. 计算压缩缓冲区大小
    unsigned long ulCompressedLength = ZSTD_compressBound(ulOriginalLength);
    
    // 2. 压缩
    int iRet = compress(CompressedBuffer, &ulCompressedLength, (PBYTE)szBuffer, ulOriginalLength);
    
    // 3. 获取加密的头部
    HeaderFlag H = m_Encoder->GetHead();
    
    // 4. 对压缩数据进行编码
    m_Encoder->Encode(CompressedBuffer, ulCompressedLength, (LPBYTE)H.data());
    
    // 5. 构建数据包
    m_WriteBuffer.WriteBuffer((PBYTE)H.data(), m_Encoder->GetFlagLen());      // 头部标识
    m_WriteBuffer.WriteBuffer((PBYTE)&ulPackTotalLength, sizeof(ULONG));     // 总长度
    m_WriteBuffer.WriteBuffer((PBYTE)&ulOriginalLength, sizeof(ULONG));      // 原始长度
    m_WriteBuffer.WriteBuffer(CompressedBuffer, ulCompressedLength);         // 压缩数据
    
    // 6. 伪装并分块发送
    return SendWithSplit((char*)m_WriteBuffer.GetBuffer(), m_WriteBuffer.GetBufferLength(), MAX_SEND_BUFFER, cmd, mask);
}
```

### 4.2 接收流程 (服务端 → 客户端)

```
接收数据 ──► 去伪装 ──► 验证头部 ──► 头部解密 ──► 数据体解密 ──► 解压 ──► 处理数据
         (UnMask)  (CheckHead) (Header Dec) (Decode)   (Decompress)
```

**详细步骤**:

1. **去伪装 (UnMask)**: 移除HTTP包装 (如有)
2. **验证头部 (CheckHead)**: 尝试所有7种解密方法识别协议
3. **头部解密 (Header Decrypt)**: 使用匹配的方法解密头部
4. **提取长度信息**: 读取压缩长度和原始长度
5. **数据体解密 (Decode)**: XOREncoder16解密压缩数据
6. **解压 (Decompress)**: ZSTD/ZLIB解压
7. **处理数据**: 回调通知上层处理

**代码实现** (`server/2015Remote/IOCPServer.cpp` - `ParseReceivedData` 函数):

```cpp
BOOL ParseReceivedData(CONTEXT_OBJECT* ContextObject, DWORD dwTrans, pfnNotifyProc m_NotifyProc, ...)
{
    // 1. 去伪装
    ULONG ret = TryUnMask(src, srcSize, maskType);
    
    // 2. 验证并解密头部 (尝试所有方法)
    HeaderEncType encType = HeaderEncUnknown;
    FlagType flagType = CheckHead(szPacketFlag, encType);
    
    // 3. 读取长度信息
    ULONG ulCompressedLength = ...;
    ULONG ulOriginalLength = ...;
    
    // 4. 数据体解密
    ContextObject->Decode(CompressedBuffer, ulOriginalLength);
    
    // 5. 解压
    size_t iRet = Muncompress(DeCompressedBuffer, &ulOriginalLength, CompressedBuffer, ulCompressedLength);
    
    // 6. 通知处理
    m_NotifyProc(ContextObject);
}
```

### 4.3 处理顺序总结

| 方向 | 步骤1 | 步骤2 | 步骤3 | 步骤4 |
|-----|-------|-------|-------|-------|
| **发送** | 编码原始数据 | **压缩** | **加密** | 伪装发送 |
| **接收** | 去伪装 | **解密** | **解压** | 解码处理 |

**关键原则**: 
- 发送时: **先压缩后加密**
- 接收时: **先解密后解压**

---

## 5. 客户端加入列表流程

### 5.1 C++ 服务端流程 (IOCPServer)

```
Accept连接 → 分配Context → 绑定完成端口 → 投递初始化 → 接收登录包 → 通知回调 → 加入列表
```

**详细代码流程**:

#### 步骤1: 接受连接 (`OnAccept`)

```cpp
void IOCPServer::OnAccept()
{
    // 1. 接受连接
    SOCKET sClientSocket = accept(m_sListenSocket, (sockaddr*)&ClientAddr, &iLen);
    
    // 2. 分配Context对象
    PCONTEXT_OBJECT ContextObject = AllocateContext(sClientSocket);
    
    // 3. 绑定到完成端口
    CreateIoCompletionPort((HANDLE)sClientSocket, m_hCompletionPort, (ULONG_PTR)ContextObject, 0);
    
    // 4. 设置KeepAlive
    setsockopt(sClientSocket, SOL_SOCKET, SO_KEEPALIVE, ...);
    
    // 5. 加入连接列表
    m_ContextConnectionList.AddTail(ContextObject);
    
    // 6. 投递初始化请求
    PostQueuedCompletionStatus(m_hCompletionPort, 0, (ULONG_PTR)ContextObject, &OverlappedPlus->m_ol);
    
    // 7. 投递接收请求
    PostRecv(ContextObject);
}
```

#### 步骤2: 分配Context (`AllocateContext`)

```cpp
PCONTEXT_OBJECT IOCPServer::AllocateContext(SOCKET s)
{
    // 检查连接数限制 (默认10000)
    if (m_ContextConnectionList.GetCount() >= m_ulMaxConnections) {
        return NULL;
    }
    
    // 从空闲池获取或新建
    ContextObject = !m_ContextFreePoolList.IsEmpty() ? 
                    m_ContextFreePoolList.RemoveHead() : 
                    new CONTEXT_OBJECT;
    
    // 初始化成员
    ContextObject->InitMember(s, this);
    
    return ContextObject;
}
```

#### 步骤3: 处理登录 (`ParseReceivedData`)

```cpp
// 当收到TOKEN_LOGIN包时
if (m_NotifyProc(ContextObject))
    ret = DeCompressedBuffer[0] == TOKEN_LOGIN ? 999 : 1;
```

#### 步骤4: UI通知回调

通过 `m_NotifyProc` 回调函数通知主窗口，主窗口将客户端信息插入列表控件。

### 5.2 Go 服务端流程 (connection/manager.go)

```go
// 添加新连接
func (m *Manager) Add(ctx *Context) error {
    // 1. 检查最大连接数
    if int(m.count.Load()) >= m.maxConns {
        return ErrMaxConnections
    }
    
    // 2. 分配唯一ID
    ctx.ID = m.idCounter.Add(1)
    
    // 3. 存入连接映射
    m.connections.Store(ctx.ID, ctx)
    m.count.Add(1)
    
    // 4. 触发连接回调
    if m.onConnect != nil {
        m.onConnect(ctx)
    }
    
    return nil
}
```

### 5.3 客户端信息结构

```cpp
// server/2015Remote/context.h
enum {
    ONLINELIST_IP = 0,          // IP
    ONLINELIST_ADDR,            // 地址
    ONLINELIST_LOCATION,        // 物理位置
    ONLINELIST_COMPUTER_NAME,   // 计算机名/备注
    ONLINELIST_OS,              // 操作系统
    ONLINELIST_CPU,             // CPU
    ONLINELIST_VIDEO,           // 摄像头
    ONLINELIST_PING,            // PING延迟
    ONLINELIST_VERSION,         // 版本信息
    ONLINELIST_INSTALLTIME,     // 安装时间
    ONLINELIST_LOGINTIME,       // 上线时间
    ONLINELIST_CLIENTTYPE,      // 客户端类型
    ONLINELIST_PATH,            // 文件路径
    ONLINELIST_PUBIP,           // 公网IP
    ONLINELIST_MAX,
};
```

### 5.4 连接生命周期

```
新连接 → AllocateContext → 加入ConnectionList → 处理请求 → 断开 → RemoveStaleContext → 回收到FreePoolList
```

**内存池机制**: 断开的Context对象不会立即销毁，而是回收到 `m_ContextFreePoolList` 以供复用，减少内存分配开销。

---

## 6. 代码文件索引

| 功能 | 文件路径 |
|-----|---------|
| 协议头定义与加密 | `common/header.h` |
| 头部加密算法V1-V6 | `common/encfuncs.h` |
| 数据体编码器 | `common/encrypt.h` |
| 数据包伪装 | `common/mask.h` |
| AES加密实现 | `common/aes.c`, `common/aes.h` |
| ZSTD压缩封装 | `common/zstd_wrapper.c`, `common/zstd_wrapper.h` |
| C++客户端通讯 | `client/IOCPClient.cpp`, `client/IOCPClient.h` |
| C++服务端通讯 | `server/2015Remote/IOCPServer.cpp`, `server/2015Remote/IOCPServer.h` |
| Context定义 | `server/2015Remote/context.h` |
| Go服务端协议 | `server/go/protocol/header.go`, `server/go/protocol/codec.go` |
| Go连接管理 | `server/go/connection/manager.go`, `server/go/connection/context.go` |

---

## 附录: 需要加密/压缩的场景汇总

### 需要加密的场景

| 场景 | 加密类型 | 说明 |
|-----|---------|-----|
| HELL协议数据包 | Header V0-V7 + XOREncoder16 | 主要协议，完整加密 |
| HELLO协议数据包 | Header V0-V7 + XOREncoder16 | 与HELL类似 |
| Shine协议数据包 | 仅Header加密 | 简单协议 |
| FUCK协议数据包 | Header加密 | 备用协议 |
| AES模式 | AES-CBC-128 | 当param[7]==1时启用 |

### 需要压缩的场景

| 场景 | 压缩方法 | 说明 |
|-----|---------|-----|
| 所有TCP数据传输 | ZSTD (默认) | 主要压缩算法 |
| 兼容旧客户端 | ZLIB | 自动降级 |
| 小数据包 (<12字节) | 无压缩 | 避免压缩开销 |
| 大数据包 (>1MB) | ZSTD多线程 | 提高性能 |

---

*文档生成时间: 2026-01-30*
*版本: 1.0*
