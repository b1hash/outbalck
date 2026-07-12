# 下级功能限制方案设计

## 一、背景

当前的 UI 定制化（UIBranding.h）是编译时配置，适用于开发商自己定制品牌。但在生成下级主控时，上级可能希望限制下级可用的功能（如禁止下级使用"生成Master"功能），这需要运行时配置机制。

### 需求场景

```
超级管理员（完整功能）
    │
    ├─→ 生成主控A（禁用：钱包、生成Master）
    │       │
    │       └─→ 主控A的下级（继承禁用 + 可能更多限制）
    │
    └─→ 生成主控B（禁用：授权管理、ShellCode）
```

### 设计目标

1. 生成主控时可选择禁用哪些功能
2. 功能限制固化到生成的程序中
3. 下级只能继承限制，不能解除
4. 与现有 UIBranding 编译时配置兼容

---

## 二、技术方案

### 2.1 存储位置

复用现有的 `g_UpperHash` 数组未使用空间，无需新增标记区域。

```
g_UpperHash[260] 当前布局:
├─ [0-99]    MASTER_HASH_STR 标记（用于搜索定位）
├─ [100-163] 64字节上级哈希
└─ [164-259] 未使用（96字节）  ← 用于存储功能标志
```

### 2.2 扩展布局

```
g_UpperHash[260] 扩展布局:
├─ [0-99]    MASTER_HASH_STR 标记
├─ [100-163] 64字节上级哈希
├─ [164-167] 版本标识 "FLG1"
├─ [168-175] MenuFlags     主菜单位图 (64位，支持64个菜单项)
├─ [176-183] ToolbarFlags  工具栏位图 (64位)
├─ [184-191] ContextFlags  右键菜单位图 (64位)
└─ [192-259] 预留 (68字节)
```

### 2.3 向后兼容

| 场景 | [164-167] 值 | 行为 |
|------|-------------|------|
| 旧版本程序 | 全零 | 无运行时限制，仅编译时配置生效 |
| 新版本 + 无限制 | "FLG1" + 全零位图 | 所有功能可用 |
| 新版本 + 有限制 | "FLG1" + 非零位图 | 按位图隐藏功能 |

---

## 三、数据结构

### 3.1 功能标志结构体

```cpp
// FeatureFlags.h

#pragma once
#include <stdint.h>

#define FEATURE_FLAGS_VERSION   "FLG1"
#define FEATURE_FLAGS_OFFSET    164     // g_UpperHash 中的偏移

#pragma pack(push, 1)
typedef struct FeatureFlags {
    char     Version[4];      // "FLG1" 版本标识
    uint64_t MenuFlags;       // 主菜单位图 (64位，支持64个菜单项)
    uint64_t ToolbarFlags;    // 工具栏位图 (64位)
    uint64_t ContextFlags;    // 右键菜单位图 (64位)
    char     Reserved[68];    // 预留扩展
} FeatureFlags;  // 4 + 8 + 8 + 8 + 68 = 96 bytes
#pragma pack(pop)
```

### 3.2 位图定义（完整映射表）

位图定义与 UIBranding.h 中的宏一一对应，便于实施时查找。

```cpp
// ===== MenuFlags 位定义 (uint64_t) =====
// 与 UIBranding.h 中 HIDE_MENU_* 宏对应

// 文件菜单 [0-3]
#define MF_SETTINGS           (1ULL << 0)   // HIDE_MENU_SETTINGS
#define MF_NOTIFY_SETTINGS    (1ULL << 1)   // HIDE_MENU_NOTIFY_SETTINGS
#define MF_WALLET             (1ULL << 2)   // HIDE_MENU_WALLET
#define MF_NETWORK            (1ULL << 3)   // HIDE_MENU_NETWORK

// 工具菜单 [4-10]
#define MF_INPUT_PASSWORD     (1ULL << 4)   // HIDE_MENU_INPUT_PASSWORD
#define MF_IMPORT_LICENSE     (1ULL << 5)   // HIDE_MENU_IMPORT_LICENSE
#define MF_RCEDIT             (1ULL << 6)   // HIDE_MENU_RCEDIT
#define MF_GEN_AUTH           (1ULL << 7)   // HIDE_MENU_GEN_AUTH
#define MF_GEN_MASTER         (1ULL << 8)   // HIDE_MENU_GEN_MASTER
#define MF_LICENSE_MGR        (1ULL << 9)   // HIDE_MENU_LICENSE_MGR
#define MF_V2_PRIVATEKEY      (1ULL << 10)  // HIDE_MENU_V2_PRIVATEKEY

// ShellCode子菜单 [11-19]
#define MF_SHELLCODE_C        (1ULL << 11)  // HIDE_MENU_SHELLCODE_C
#define MF_SHELLCODE_BIN      (1ULL << 12)  // HIDE_MENU_SHELLCODE_BIN
#define MF_SHELLCODE_LOAD_TEST (1ULL << 13) // HIDE_MENU_SHELLCODE_LOAD_TEST
#define MF_SHELLCODE_OBFS     (1ULL << 14)  // HIDE_MENU_SHELLCODE_OBFS
#define MF_SHELLCODE_OBFS_BIN (1ULL << 15)  // HIDE_MENU_SHELLCODE_OBFS_BIN
#define MF_SHELLCODE_OBFS_TEST (1ULL << 16) // HIDE_MENU_SHELLCODE_OBFS_TEST
#define MF_SHELLCODE_AES_C    (1ULL << 17)  // HIDE_MENU_SHELLCODE_AES_C
#define MF_SHELLCODE_AES_BIN  (1ULL << 18)  // HIDE_MENU_SHELLCODE_AES_BIN
#define MF_SHELLCODE_AES_TEST (1ULL << 19)  // HIDE_MENU_SHELLCODE_AES_TEST

// 参数菜单 [20-27]
#define MF_KBLOGGER           (1ULL << 20)  // HIDE_MENU_KBLOGGER
#define MF_LOGIN_NOTIFY       (1ULL << 21)  // HIDE_MENU_LOGIN_NOTIFY
#define MF_ENABLE_LOG         (1ULL << 22)  // HIDE_MENU_ENABLE_LOG
#define MF_PRIVACY_WALLPAPER  (1ULL << 23)  // HIDE_MENU_PRIVACY_WALLPAPER
#define MF_FILE_V2            (1ULL << 24)  // HIDE_MENU_FILE_V2
#define MF_HOOK_WIN           (1ULL << 25)  // HIDE_MENU_HOOK_WIN
#define MF_RUN_AS_SERVICE     (1ULL << 26)  // HIDE_MENU_RUN_AS_SERVICE
#define MF_RUN_AS_USER        (1ULL << 27)  // HIDE_MENU_RUN_AS_USER

// 扩展菜单 [28-37]
#define MF_HISTORY_CLIENTS    (1ULL << 28)  // HIDE_MENU_HISTORY_CLIENTS
#define MF_BACKUP_DATA        (1ULL << 29)  // HIDE_MENU_BACKUP_DATA
#define MF_IMPORT_DATA        (1ULL << 30)  // HIDE_MENU_IMPORT_DATA
#define MF_RELOAD_PLUGINS     (1ULL << 31)  // HIDE_MENU_RELOAD_PLUGINS
#define MF_PLUGIN_REQUEST     (1ULL << 32)  // HIDE_MENU_PLUGIN_REQUEST
#define MF_FRPS_FOR_SUB       (1ULL << 33)  // HIDE_MENU_FRPS_FOR_SUB
#define MF_CHANGE_LANG        (1ULL << 34)  // HIDE_MENU_CHANGE_LANG
#define MF_CHOOSE_LANG_DIR    (1ULL << 35)  // HIDE_MENU_CHOOSE_LANG_DIR
#define MF_LOCATION_QQWRY     (1ULL << 36)  // HIDE_MENU_LOCATION_QQWRY
#define MF_LOCATION_IP2REGION (1ULL << 37)  // HIDE_MENU_LOCATION_IP2REGION

// 帮助菜单 [38-42]
#define MF_IMPORTANT          (1ULL << 38)  // HIDE_MENU_IMPORTANT
#define MF_FEEDBACK           (1ULL << 39)  // HIDE_MENU_FEEDBACK
#define MF_WHAT_IS_THIS       (1ULL << 40)  // HIDE_MENU_WHAT_IS_THIS
#define MF_MASTER_TRAIL       (1ULL << 41)  // HIDE_MENU_MASTER_TRAIL
#define MF_REQUEST_AUTH       (1ULL << 42)  // HIDE_MENU_REQUEST_AUTH

// [43-63] 预留


// ===== ToolbarFlags 位定义 (uint64_t) =====
// 与 UIBranding.h 中 HIDE_TOOLBAR_* 宏对应，按索引顺序

#define TF_TERMINAL           (1ULL << 0)   // HIDE_TOOLBAR_TERMINAL
#define TF_PROCESS            (1ULL << 1)   // HIDE_TOOLBAR_PROCESS
#define TF_WINDOW             (1ULL << 2)   // HIDE_TOOLBAR_WINDOW
#define TF_DESKTOP            (1ULL << 3)   // HIDE_TOOLBAR_DESKTOP
#define TF_FILE               (1ULL << 4)   // HIDE_TOOLBAR_FILE
#define TF_AUDIO              (1ULL << 5)   // HIDE_TOOLBAR_AUDIO
#define TF_VIDEO              (1ULL << 6)   // HIDE_TOOLBAR_VIDEO
#define TF_SERVICE            (1ULL << 7)   // HIDE_TOOLBAR_SERVICE
#define TF_REGISTER           (1ULL << 8)   // HIDE_TOOLBAR_REGISTER
#define TF_KEYBOARD           (1ULL << 9)   // HIDE_TOOLBAR_KEYBOARD
#define TF_SETTINGS           (1ULL << 10)  // HIDE_TOOLBAR_SETTINGS
#define TF_BUILD              (1ULL << 11)  // HIDE_TOOLBAR_BUILD
#define TF_SEARCH             (1ULL << 12)  // HIDE_TOOLBAR_SEARCH
#define TF_HELP               (1ULL << 13)  // HIDE_TOOLBAR_HELP

// [14-63] 预留


// ===== ContextFlags 位定义 (uint64_t) =====
// 与 UIBranding.h 中 HIDE_CTX_* 宏对应

// 在线列表右键菜单 [0-16]
#define CF_MESSAGE            (1ULL << 0)   // HIDE_CTX_MESSAGE
#define CF_UPDATE             (1ULL << 1)   // HIDE_CTX_UPDATE
#define CF_DELETE             (1ULL << 2)   // HIDE_CTX_DELETE
#define CF_SHARE              (1ULL << 3)   // HIDE_CTX_SHARE
#define CF_PROXY              (1ULL << 4)   // HIDE_CTX_PROXY
#define CF_HOSTNOTE           (1ULL << 5)   // HIDE_CTX_HOSTNOTE
#define CF_VIRTUAL_DESKTOP    (1ULL << 6)   // HIDE_CTX_VIRTUAL_DESKTOP
#define CF_GRAY_DESKTOP       (1ULL << 7)   // HIDE_CTX_GRAY_DESKTOP
#define CF_REMOTE_DESKTOP     (1ULL << 8)   // HIDE_CTX_REMOTE_DESKTOP
#define CF_H264_DESKTOP       (1ULL << 9)   // HIDE_CTX_H264_DESKTOP
#define CF_AUTHORIZE          (1ULL << 10)  // HIDE_CTX_AUTHORIZE
#define CF_UNAUTHORIZE        (1ULL << 11)  // HIDE_CTX_UNAUTHORIZE
#define CF_ASSIGN_TO          (1ULL << 12)  // HIDE_CTX_ASSIGN_TO
#define CF_ADD_WATCH          (1ULL << 13)  // HIDE_CTX_ADD_WATCH
#define CF_LOGIN_NOTIFY       (1ULL << 14)  // HIDE_CTX_LOGIN_NOTIFY
#define CF_RUN_AS_ADMIN       (1ULL << 15)  // HIDE_CTX_RUN_AS_ADMIN
#define CF_UNINSTALL          (1ULL << 16)  // HIDE_CTX_UNINSTALL
#define CF_PRIVATE_SCREEN     (1ULL << 17)  // HIDE_CTX_PRIVATE_SCREEN
#define CF_REGROUP            (1ULL << 18)  // HIDE_CTX_REGROUP
#define CF_INJ_NOTEPAD        (1ULL << 19)  // HIDE_CTX_INJ_NOTEPAD
#define CF_PROXY_PORT         (1ULL << 20)  // HIDE_CTX_PROXY_PORT
#define CF_PROXY_PORT_STD     (1ULL << 21)  // HIDE_CTX_PROXY_PORT_STD

// 机器管理子菜单 [22-24]
#define CF_MACHINE_SHUTDOWN   (1ULL << 22)  // HIDE_CTX_MACHINE_SHUTDOWN
#define CF_MACHINE_REBOOT     (1ULL << 23)  // HIDE_CTX_MACHINE_REBOOT
#define CF_MACHINE_LOGOUT     (1ULL << 24)  // HIDE_CTX_MACHINE_LOGOUT

// 执行命令子菜单 [25-28]
#define CF_EXECUTE_DOWNLOAD   (1ULL << 25)  // HIDE_CTX_EXECUTE_DOWNLOAD
#define CF_EXECUTE_UPLOAD     (1ULL << 26)  // HIDE_CTX_EXECUTE_UPLOAD
#define CF_EXECUTE_TESTRUN    (1ULL << 27)  // HIDE_CTX_EXECUTE_TESTRUN
#define CF_EXECUTE_GHOST      (1ULL << 28)  // HIDE_CTX_EXECUTE_GHOST

// [29-63] 预留
```

### 3.3 访问接口

```cpp
// 获取功能标志（返回 nullptr 表示无限制）
inline const FeatureFlags* GetFeatureFlags() {
    extern char g_UpperHash[];
    const char* ptr = g_UpperHash + FEATURE_FLAGS_OFFSET;
    if (memcmp(ptr, FEATURE_FLAGS_VERSION, 4) != 0)
        return nullptr;
    return (const FeatureFlags*)ptr;
}

// 检查菜单项是否被运行时禁用
inline bool IsMenuDisabled(uint64_t flag) {
    const FeatureFlags* ff = GetFeatureFlags();
    return ff && (ff->MenuFlags & flag);
}

// 检查工具栏按钮是否被运行时禁用
inline bool IsToolbarDisabled(uint64_t flag) {
    const FeatureFlags* ff = GetFeatureFlags();
    return ff && (ff->ToolbarFlags & flag);
}

// 检查右键菜单项是否被运行时禁用
inline bool IsContextDisabled(uint64_t flag) {
    const FeatureFlags* ff = GetFeatureFlags();
    return ff && (ff->ContextFlags & flag);
}
```

---

## 四、UI 设计

### 4.1 控件方案

由于功能项较多（43+14+24=81项），推荐使用 **Tab + CCheckListBox** 方案：

```
┌─ 下级功能限制 ────────────────────────────────────────┐
│                                                        │
│  ┌──────────┬──────────┬──────────┐                   │
│  │ 主菜单   │ 工具栏   │ 右键菜单 │                   │
│  └──────────┴──────────┴──────────┘                   │
│  ┌──────────────────────────────────────────────┐     │
│  │ ☑ 钱包                                       │ ▲   │
│  │ ☑ 生成Master                                 │ █   │
│  │ ☑ 许可证管理                                 │ █   │
│  │ ☑ V2私钥                                     │ █   │
│  │ ☑ 生成授权                                   │ █   │
│  │ ☐ PE资源编辑                                 │ █   │
│  │ ☐ 备份数据                                   │ █   │
│  │ ☐ 导入数据                                   │ ▼   │
│  └──────────────────────────────────────────────┘     │
│                                                        │
│  勾选 = 对下级隐藏   ※灰色项为上级已禁用              │
│                                                        │
│                   [ 确定 ]    [ 取消 ]                 │
└────────────────────────────────────────────────────────┘
```

### 4.2 控件说明

| 控件 | 类型 | 说明 |
|------|------|------|
| Tab控件 | CTabCtrl | 切换三类功能：主菜单/工具栏/右键菜单 |
| 列表控件 | CCheckListBox | 显示当前Tab的功能项，支持勾选 |

### 4.3 列表项状态

| 状态 | 显示 | 说明 |
|------|------|------|
| 可用 | □ 正常文字 | 可自由勾选/取消 |
| 已继承 | ☑ 灰色文字 | 上级已禁用，不可取消 |
| 已编译时隐藏 | 不显示 | UIBranding.h 中已设为1，无需再配置 |

### 4.4 交互逻辑

**默认状态**：
- 所有功能默认启用（复选框不勾选 = 下级可用）
- 上级已限制的功能显示为灰色勾选（不可取消）

**操作流程**：
1. 用户勾选要对下级**禁用**的功能
2. 点击确定，继续生成流程

---

## 五、实现步骤

### 5.1 新增文件

| 文件 | 说明 |
|------|------|
| `FeatureFlags.h` | 数据结构、位图定义、访问接口 |
| `FeatureLimitsDlg.h/cpp` | 功能限制对话框（Tab + CCheckListBox） |
| `IDD_FEATURE_LIMITS` | 对话框资源（.rc 文件中添加） |

### 5.2 修改文件

| 文件 | 修改内容 |
|------|---------|
| `2015RemoteDlg.cpp` | OnToolGenMaster 集成对话框和写入逻辑 |
| `2015RemoteDlg.cpp` | CreateSolidMenu 增加运行时检查 |
| `2015RemoteDlg.cpp` | CreateToolBar 增加运行时检查 |
| `2015RemoteDlg.cpp` | OnNMRClickOnline 增加运行时检查 |
| `generated_hash.h` | 模板更新，包含功能标志 |

### 5.3 开发顺序

```
Phase 1: 数据结构
├── 定义 FeatureFlags 结构体
├── 定义位图常量
└── 实现 GetFeatureFlags() 等接口

Phase 2: 写入逻辑
├── 修改 OnToolGenMaster
├── 在 g_UpperHash[164] 写入功能标志
└── 更新 GenerateHashHeaderFile

Phase 3: 读取逻辑
├── CreateSolidMenu 增加运行时检查
├── CreateToolBar 增加运行时检查
└── OnNMRClickOnline 增加运行时检查

Phase 4: UI 对话框
├── 创建对话框资源
└── 实现 CFeatureLimitsDlg
```

---

## 六、对话框初始化逻辑

### 6.1 列表项生成

对话框初始化时，需要根据编译时配置和继承限制来决定显示哪些项：

```cpp
void CFeatureLimitsDlg::InitMenuList()
{
    // 定义菜单项配置表
    struct MenuItem {
        const char* name;       // 显示名称
        uint64_t flag;          // 运行时标志位
        int compileMacro;       // 编译时宏值 (0或1)
    };

    static const MenuItem items[] = {
        { "钱包",         MF_WALLET,         HIDE_MENU_WALLET },
        { "生成Master",   MF_GEN_MASTER,     HIDE_MENU_GEN_MASTER },
        { "许可证管理",   MF_LICENSE_MGR,    HIDE_MENU_LICENSE_MGR },
        // ... 其他项
    };

    const FeatureFlags* inherited = GetFeatureFlags();

    for (const auto& item : items) {
        // 编译时已隐藏的功能，不显示（下级根本没有这个功能）
        if (item.compileMacro)
            continue;

        int index = m_ListMenu.AddString(item.name);

        // 上级已禁用的功能，显示为勾选+禁用状态
        // 使用高位标记继承状态（点击时检查）
        if (inherited && (inherited->MenuFlags & item.flag)) {
            m_ListMenu.SetCheck(index, TRUE);
            m_ListMenu.SetItemData(index, item.flag | 0x8000000000000000ULL);  // 高位=继承
        } else {
            m_ListMenu.SetItemData(index, item.flag);
        }
    }
}

// 在 OnCheckChange 中阻止取消继承项
void CFeatureLimitsDlg::OnCheckChange()
{
    int index = m_ListMenu.GetCaretIndex();
    if (index < 0) return;

    uint64_t data = m_ListMenu.GetItemData(index);
    if (data & 0x8000000000000000ULL) {
        // 继承项不可取消，强制保持勾选
        m_ListMenu.SetCheck(index, TRUE);
    }
}
```

### 6.2 收集用户选择

```cpp
void CFeatureLimitsDlg::OnOK()
{
    m_MenuFlags = 0;

    // 收集所有勾选项的标志位
    for (int i = 0; i < m_ListMenu.GetCount(); i++) {
        if (m_ListMenu.GetCheck(i)) {
            m_MenuFlags |= m_ListMenu.GetItemData(i);
        }
    }

    // 工具栏和右键菜单同理...

    CDialogEx::OnOK();
}
```

---

## 七、运行时检查逻辑

### 7.1 菜单隐藏（双重检查）

```cpp
// CreateSolidMenu 中
void CMy2015RemoteDlg::CreateSolidMenu()
{
    // ... 现有代码 ...

    // 编译时配置（UIBranding.h）
    #if HIDE_MENU_WALLET
        pMenu->DeleteMenu(ID_MAIN_WALLET, MF_BYCOMMAND);
    #else
        // 运行时配置（上级限制）
        if (IsMenuDisabled(MF_WALLET))
            pMenu->DeleteMenu(ID_MAIN_WALLET, MF_BYCOMMAND);
    #endif

    // 或者统一写法：
    if (HIDE_MENU_WALLET || IsMenuDisabled(MF_WALLET))
        pMenu->DeleteMenu(ID_MAIN_WALLET, MF_BYCOMMAND);
}
```

### 7.2 宏简化

```cpp
// 简化宏：编译时 OR 运行时
#define SHOULD_HIDE_MENU(compile_flag, runtime_flag) \
    ((compile_flag) || IsMenuDisabled(runtime_flag))

#define SHOULD_HIDE_TOOLBAR(compile_flag, runtime_flag) \
    ((compile_flag) || IsToolbarDisabled(runtime_flag))

#define SHOULD_HIDE_CTX(compile_flag, runtime_flag) \
    ((compile_flag) || IsContextDisabled(runtime_flag))

// 使用示例
if (SHOULD_HIDE_MENU(HIDE_MENU_WALLET, MF_WALLET))
    pMenu->DeleteMenu(ID_MAIN_WALLET, MF_BYCOMMAND);
```

---

## 八、生成流程修改

### 8.1 OnToolGenMaster 修改

```cpp
void CMy2015RemoteDlg::OnToolGenMaster()
{
    // ... 现有的密码、天数、深度输入 ...

    // === 新增：功能限制对话框 ===
    CFeatureLimitsDlg limitsDlg(this);

    // 传入当前程序的限制（用于继承显示）
    const FeatureFlags* inherited = GetFeatureFlags();
    if (inherited) {
        limitsDlg.SetInheritedFlags(inherited);
    }

    if (limitsDlg.DoModal() != IDOK)
        return;

    // ... 现有的文件读取逻辑 ...

    // 现有代码：搜索 MASTER_HASH_STR 标记（用于写入 g_UpperHash）
    char str[100] = {}, markArr[] = { MASTER_HASH_STR };
    memcpy(str, markArr, sizeof(markArr));
    int upperHashOffset = MemoryFind(curEXE, str, size, sizeof(str));

    if (upperHashOffset == -1) {
        AfxMessageBox(_T("找不到标记位置，无法生成下级主控"));
        delete[] curEXE;
        return;
    }

    {
        // 现有：写入上级哈希 [100-163]
        memcpy(curEXE + upperHashOffset + 100, masterHash.c_str(), 64);

        // === 新增：写入功能标志 [164-191] ===
        FeatureFlags flags = {};
        memcpy(flags.Version, FEATURE_FLAGS_VERSION, 4);
        flags.MenuFlags = limitsDlg.m_MenuFlags;
        flags.ToolbarFlags = limitsDlg.m_ToolbarFlags;
        flags.ContextFlags = limitsDlg.m_ContextFlags;
        memcpy(curEXE + upperHashOffset + FEATURE_FLAGS_OFFSET, &flags, sizeof(flags));
    }

    // ... 现有的保存和压缩逻辑 ...
}
```

**关键点**：功能标志写入到 `g_UpperHash` 搜索结果的偏移 164 处，与上级哈希（偏移 100）使用同一个搜索结果。

### 8.2 GenerateHashHeaderFile 更新

```cpp
// 输出功能标志注释（便于下级开发商了解限制）
headerFile << "// 功能限制标志 (位于 g_UpperHash[164-191])\n";
if (flags.MenuFlags || flags.ToolbarFlags || flags.ContextFlags) {
    headerFile << "// MenuFlags:    0x" << std::hex << std::setfill('0')
               << std::setw(16) << flags.MenuFlags << "\n";
    headerFile << "// ToolbarFlags: 0x" << std::setw(16) << flags.ToolbarFlags << "\n";
    headerFile << "// ContextFlags: 0x" << std::setw(16) << flags.ContextFlags
               << std::dec << "\n";
}
```

---

## 九、与 UIBranding 的关系

| 配置类型 | 作用对象 | 配置时机 | 可修改性 |
|---------|---------|---------|---------|
| UIBranding.h | 开发商自己 | 编译时 | 需重新编译 |
| FeatureFlags | 下级程序 | 生成时 | 固化到程序 |

**优先级**：编译时配置 > 运行时配置

如果编译时已隐藏某功能，运行时配置无效（已经没有该功能）。运行时配置只能在编译时未隐藏的功能中进一步限制。

---

## 十、测试要点

1. **向后兼容**：旧版本程序（无 FLG1 标记）正常运行
2. **继承正确**：下级无法解除上级的限制
3. **位图准确**：每个功能对应正确的位
4. **UI 响应**：勾选/取消正确反映到位图
5. **双重检查**：编译时和运行时配置均生效
