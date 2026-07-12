# YAMA 定制化开发指南

> 面向技术型客户的二次开发与品牌定制

---

## 目标读者

- 有一定编程基础的客户
- 希望打造自己品牌的服务商
- 承接远程解决方案外包的开发者

---

## 第一部分：定制化概述

### 1. 可定制内容

| 类别 | 可定制项 | 难度 | 需要重编译 |
|------|---------|------|-----------|
| 品牌外观 | 程序名称、图标、版本信息 | 简单 | 是 |
| 菜单控制 | 隐藏/显示特定菜单项 | 简单 | 是 |
| 工具栏 | 隐藏/显示特定按钮 | 简单 | 是 |
| 右键菜单 | 隐藏/显示特定功能 | 简单 | 是 |
| 界面文字 | 修改翻译/添加语言 | 中等 | 否 |
| 功能定制 | 添加新功能、修改行为 | 高级 | 是 |

### 2. 定制方式

#### 2.1 配置文件定制（推荐）

修改 `UIBranding.h` 文件重新编译。

**适合场景：**
- 品牌名称修改
- 隐藏不需要的功能
- 快速生成定制版本

#### 2.2 资源替换定制

替换图标、图片等资源文件。

**适合场景：**
- 更换程序图标
- 自定义启动画面

#### 2.3 语言文件定制

修改语言文件实现界面文字定制。

**适合场景：**
- 修改界面用语
- 添加新语言支持

#### 2.4 源码级定制

修改源代码实现深度定制。

**适合场景：**
- 添加新功能
- 修改现有行为
- 深度集成

---

## 第二部分：品牌定制

### 3. UIBranding.h 配置文件

品牌定制的核心是修改 `server/2015Remote/UIBranding.h` 文件。

**文件位置：** `server/2015Remote/UIBranding.h`

**编码要求：** UTF-8 with BOM（MSVC 要求）

#### 3.1 快速定制（4 项核心设置）

最少只需修改以下 4 项即可完成基本品牌定制：

```cpp
// 程序名称（窗口标题）
#define BRAND_APP_NAME          "YourBrand"

// 启动画面 Logo（建议大写）
#define BRAND_SPLASH_NAME       "YOURBRAND"

// 托盘图标提示
#define BRAND_TRAY_TIP          "YourBrand 远程管理"

// 版权信息
#define BRAND_COPYRIGHT         "Copyright (C) 2024 YourCompany"
```

#### 3.2 完整品牌设置

| 配置项 | 说明 | 字符限制 |
|--------|------|---------|
| `BRAND_APP_NAME` | 窗口标题、关于对话框 | 可用中文 |
| `BRAND_VERSION` | 版本号（建议 X.Y.Z 格式） | 可用中文 |
| `BRAND_SPLASH_NAME` | 启动画面 Logo 文字 | 可用中文 |
| `BRAND_TRAY_TIP` | 托盘图标悬停提示 | 可用中文 |
| `BRAND_COPYRIGHT` | 版权声明 | 可用中文 |
| `BRAND_SERVICE_DISPLAY` | Windows 服务显示名 | 可用中文 |
| `BRAND_LICENSE_DESC` | 许可证文件描述 | 可用中文 |

#### 3.3 文件和系统标识

以下配置有字符限制，仅支持 ASCII：

| 配置项 | 说明 | 限制 |
|--------|------|------|
| `BRAND_DUMP_PREFIX` | 崩溃转储文件前缀 | 仅 ASCII |
| `BRAND_EXE_NAME` | 默认 EXE 文件名 | 仅 ASCII |
| `BRAND_LICENSE_PREFIX` | 许可证文件前缀 | 仅 ASCII |
| `BRAND_DB_NAME` | 数据库文件名 | 仅 ASCII |
| `BRAND_DATA_FOLDER` | 数据目录名 | 仅 ASCII |
| `BRAND_SERVICE_NAME` | Windows 服务名（无空格） | 仅 ASCII |
| `BRAND_REGISTRY_KEY` | 注册表键名（无空格） | 仅 ASCII |

**示例：**

```cpp
#define BRAND_DUMP_PREFIX       "MyRemote"
#define BRAND_EXE_NAME          "MyRemote.exe"
#define BRAND_LICENSE_PREFIX    "MyRemote"
#define BRAND_SERVICE_NAME      "MyRemoteService"
#define BRAND_REGISTRY_KEY      "MyRemote"
```

### 4. 隐藏菜单项

通过设置宏为 `1` 可以隐藏对应菜单项。

> **完整列表**：以下仅列出常用配置项，完整列表请查看 `UIBranding.h` 文件。

#### 4.1 主菜单项（常用）

```cpp
// 文件菜单
#define HIDE_MENU_SETTINGS              0   // 参数设置
#define HIDE_MENU_WALLET                0   // 钱包
#define HIDE_MENU_NETWORK               0   // 网络

// 工具菜单
#define HIDE_MENU_INPUT_PASSWORD        0   // 输入密码
#define HIDE_MENU_IMPORT_LICENSE        0   // 导入许可证
#define HIDE_MENU_GEN_AUTH              0   // 生成授权
#define HIDE_MENU_GEN_MASTER            0   // 生成Master
#define HIDE_MENU_LICENSE_MGR           0   // 许可证管理

// 参数菜单
#define HIDE_MENU_KBLOGGER              0   // 键盘记录
#define HIDE_MENU_LOGIN_NOTIFY          0   // 登录通知

// 扩展菜单
#define HIDE_MENU_FRPS_FOR_SUB          0   // 下级FRP
```

#### 4.2 使用示例

隐藏高级功能，创建简化版本：

```cpp
// 隐藏 ShellCode 相关功能
#define HIDE_MENU_SHELLCODE_C           1
#define HIDE_MENU_SHELLCODE_BIN         1
#define HIDE_MENU_SHELLCODE_LOAD_TEST   1
#define HIDE_MENU_SHELLCODE_OBFS        1
#define HIDE_MENU_SHELLCODE_AES_C       1
#define HIDE_MENU_SHELLCODE_AES_BIN     1

// 隐藏开发相关功能
#define HIDE_MENU_V2_PRIVATEKEY         1
#define HIDE_MENU_RCEDIT                1
```

### 5. 隐藏工具栏按钮（常用）

```cpp
#define HIDE_TOOLBAR_TERMINAL           0   // 终端管理
#define HIDE_TOOLBAR_PROCESS            0   // 进程管理
#define HIDE_TOOLBAR_DESKTOP            0   // 桌面管理
#define HIDE_TOOLBAR_FILE               0   // 文件管理
#define HIDE_TOOLBAR_AUDIO              0   // 语音管理
#define HIDE_TOOLBAR_VIDEO              0   // 视频管理
#define HIDE_TOOLBAR_SERVICE            0   // 服务管理
#define HIDE_TOOLBAR_REGISTER           0   // 注册表管理
#define HIDE_TOOLBAR_KEYBOARD           0   // 键盘记录
#define HIDE_TOOLBAR_SETTINGS           0   // 参数设置
#define HIDE_TOOLBAR_BUILD              0   // 生成服务端
```

### 6. 隐藏右键菜单项（常用）

```cpp
// 远程桌面选项
#define HIDE_CTX_VIRTUAL_DESKTOP        0   // 虚拟桌面
#define HIDE_CTX_GRAY_DESKTOP           0   // 灰度桌面
#define HIDE_CTX_REMOTE_DESKTOP         0   // 远程桌面
#define HIDE_CTX_H264_DESKTOP           0   // H264桌面

// 系统操作
#define HIDE_CTX_MACHINE_SHUTDOWN       0   // 关机
#define HIDE_CTX_MACHINE_REBOOT         0   // 重启
#define HIDE_CTX_MACHINE_LOGOUT         0   // 注销

// 执行命令
#define HIDE_CTX_EXECUTE_DOWNLOAD       0   // 下载执行
#define HIDE_CTX_EXECUTE_UPLOAD         0   // 上传执行
```

---

## 第三部分：视觉定制

### 7. 图标替换

#### 7.1 图标文件位置

图标资源位于 `server/2015Remote/res/` 目录：

| 文件 | 用途 | 尺寸要求 |
|------|------|---------|
| `2015Remote.ico` | 主程序图标 | 多尺寸 ICO |
| `tray_online.ico` | 托盘在线图标 | 16x16, 32x32 |
| `tray_offline.ico` | 托盘离线图标 | 16x16, 32x32 |

#### 7.2 图标格式要求

ICO 文件应包含多种尺寸：
- 16x16（小图标、托盘）
- 32x32（标准图标）
- 48x48（大图标）
- 256x256（高清图标）

**推荐工具：**
- IcoFX
- GIMP
- 在线 ICO 生成器

#### 7.3 替换步骤

1. 准备新图标文件
2. 备份原有图标
3. 将新图标复制到 `res/` 目录，保持文件名一致
4. 重新编译程序

### 8. 启动画面定制

启动画面的 Logo 文字由 `BRAND_SPLASH_NAME` 配置：

```cpp
#define BRAND_SPLASH_NAME       "YOURBRAND"
```

**建议：**
- 使用大写字母，视觉效果更好
- 保持简短（建议 8 个字符以内）
- 可以使用中文

---

## 第四部分：界面文字定制

### 9. 语言文件机制

YAMA 使用 INI 格式的语言文件实现多语言支持。

#### 9.1 语言文件位置

**源代码位置：**
```
server/2015Remote/lang/
├── en_US.ini      # 英文翻译
├── zh_TW.ini      # 繁体中文翻译
└── (其他语言)
```

**运行时位置：** 程序会从程序同目录的 `lang/` 文件夹读取语言文件。

**发布时：** 将 `lang/` 文件夹与程序放在同一目录。

#### 9.2 文件格式

```ini
; 注释行
简体中文原文=翻译文本
另一条原文=另一条翻译
```

**编码要求：** GB2312（不是 UTF-8！）

#### 9.3 添加翻译

由于语言文件必须使用 GB2312 编码，建议使用以下方法添加翻译：

**方法 A：使用支持 GB2312 编码的编辑器**

1. 使用 Notepad++、VS Code（安装编码插件）等编辑器
2. 打开语言文件，确认编码为 GB2312
3. 添加新的翻译条目
4. 保存时确保保持 GB2312 编码

**方法 B：使用 PowerShell 命令**

```powershell
# 追加单条翻译
$gb2312 = [System.Text.Encoding]::GetEncoding('GB2312')
$content = "新功能=New Feature`r`n"
[System.IO.File]::AppendAllText('lang\en_US.ini', $content, $gb2312)
```

> **注意**：直接用记事本编辑可能导致编码错误，请使用专业编辑器。

### 10. 创建新语言包

1. 复制 `en_US.ini` 作为模板
2. 重命名为新语言代码（如 `ja_JP.ini`）
3. 翻译所有条目
4. 保存为 GB2312 编码

---

## 第五部分：开发环境

### 11. 环境要求

| 组件 | 版本要求 |
|------|---------|
| Visual Studio | 2019 或 2022 |
| Windows SDK | 10.0.19041.0 或更高 |
| C++ 标准 | C++17 |
| 工具集 | v142 (MSVC v142) |
| MFC 库 | C++ MFC for v142 build tools |

### 12. 项目结构

```
SimpleRemoter/
├── server/2015Remote/    # 主控程序源码
│   ├── UIBranding.h      # 品牌配置
│   ├── 2015RemoteDlg.cpp # 主对话框
│   ├── SettingDlg.cpp    # 设置页面
│   ├── BuildDlg.cpp      # 生成对话框
│   ├── res/              # 资源文件
│   └── lang/             # 语言文件
├── client/               # 受管端源码
├── common/               # 公共代码
├── linux/                # Linux 客户端
└── Bin/                  # 编译输出
```

### 13. 关键文件说明

| 文件 | 功能 |
|------|------|
| `UIBranding.h` | 品牌和功能开关配置 |
| `2015RemoteDlg.cpp` | 主窗口、菜单、状态栏 |
| `SettingDlg.cpp` | 设置对话框 |
| `BuildDlg.cpp` | 生成受管程序对话框 |
| `CPasswordDlg.cpp` | 授权生成对话框 |
| `FrpsForSubDlg.cpp` | FRP 代理配置 |
| `ScreenSpyDlg.cpp` | 远程桌面窗口 |
| `FileManagerDlg.cpp` | 文件管理窗口 |
| `TerminalDlg.cpp` | 远程终端窗口 |

---

## 第六部分：编译与发布

### 14. 编译脚本

使用提供的编译脚本：

```powershell
# 完整编译（先编译依赖项，再编译主控）
.\build.ps1

# 仅编译主控程序（跳过依赖项，适合快速重编译）
.\build.ps1 -ServerOnly

# 发版模式（重编译所有 + UPX 压缩）
.\build.ps1 -Publish

# Debug 模式
.\build.ps1 -Config Debug

# 编译 32 位和 64 位
.\build.ps1 -Platform all

# 清理后重编译
.\build.ps1 -Clean
```

### 15. 编译输出

| 输出文件 | 位置 |
|---------|------|
| 64 位主控 | `Bin/Yama_x64.exe` |
| 32 位主控 | `Bin/Yama_x86.exe` |

### 16. 发布准备

#### 16.1 发布检查清单

- [ ] 修改 `UIBranding.h` 中的品牌信息
- [ ] 替换程序图标（如需要）
- [ ] 使用 Release 配置编译
- [ ] 使用 `-Publish` 进行 UPX 压缩
- [ ] 测试所有功能正常
- [ ] 测试授权导入功能
- [ ] 测试受管程序生成

#### 16.2 文件清单

发布时需要包含：

```
YourBrand/
├── YourBrand.exe     # 主程序（重命名）
└── lang/             # 语言文件（可选）
    ├── en_US.ini
    └── zh_TW.ini
```

---

## 第七部分：常见定制场景

### 17. 场景一：简单品牌替换

**需求：** 更换程序名称和图标

**步骤：**
1. 修改 `UIBranding.h` 中的品牌名称
2. 替换 `res/` 目录中的图标文件
3. 编译：`.\build.ps1 -Publish`
4. 重命名输出文件

### 18. 场景二：功能精简版

**需求：** 创建只保留基本功能的精简版

**步骤：**
1. 在 `UIBranding.h` 中隐藏高级功能：

```cpp
// 隐藏开发相关
#define HIDE_MENU_SHELLCODE_C           1
#define HIDE_MENU_SHELLCODE_BIN         1
#define HIDE_MENU_V2_PRIVATEKEY         1
#define HIDE_MENU_RCEDIT                1

// 隐藏高级功能
#define HIDE_TOOLBAR_AUDIO              1
#define HIDE_TOOLBAR_VIDEO              1
#define HIDE_TOOLBAR_REGISTER           1
#define HIDE_TOOLBAR_KEYBOARD           1
```

2. 编译发布

### 19. 场景三：多品牌版本

**需求：** 为不同客户生成不同品牌的版本

**方案：**
1. 创建多个 `UIBranding.h` 副本（如 `UIBranding_ClientA.h`）
2. 编译前复制对应配置文件覆盖 `UIBranding.h`
3. 编译并重命名输出
4. 或编写脚本自动化此过程

---

## 第八部分：最佳实践

### 20. 版本管理建议

#### 20.1 分支策略

```
main (或 master)        # 上游原版
├── custom/base         # 基础定制
├── custom/clientA      # 客户 A 定制
└── custom/clientB      # 客户 B 定制
```

#### 20.2 保持同步

定期从上游合并更新：

```bash
git fetch upstream
git merge upstream/main
# 解决冲突后提交
```

### 21. 定制文档

建议记录定制内容：

| 记录项 | 说明 |
|--------|------|
| 品牌配置 | 修改了哪些 `UIBranding.h` 配置 |
| 隐藏功能 | 隐藏了哪些菜单/按钮 |
| 图标变更 | 替换了哪些图标文件 |
| 源码修改 | 如有源码修改，记录位置和原因 |

### 22. 测试清单

每次定制后测试：

- [ ] 程序能正常启动
- [ ] 品牌信息显示正确
- [ ] 图标显示正确
- [ ] 授权导入功能正常
- [ ] 受管程序生成正常
- [ ] 远程桌面功能正常
- [ ] 文件管理功能正常
- [ ] 隐藏的功能确实不显示

---

## 附录

### A. 配置项速查表

| 配置项 | 文件 | 说明 |
|--------|------|------|
| 程序名称 | `UIBranding.h` | `BRAND_APP_NAME` |
| 版本号 | `UIBranding.h` | `BRAND_VERSION` |
| 启动 Logo | `UIBranding.h` | `BRAND_SPLASH_NAME` |
| 托盘提示 | `UIBranding.h` | `BRAND_TRAY_TIP` |
| 版权信息 | `UIBranding.h` | `BRAND_COPYRIGHT` |
| 隐藏菜单 | `UIBranding.h` | `HIDE_MENU_*` |
| 隐藏工具栏 | `UIBranding.h` | `HIDE_TOOLBAR_*` |
| 隐藏右键菜单 | `UIBranding.h` | `HIDE_CTX_*` |
| 界面翻译 | `lang/*.ini` | 键值对 |

### B. 资源文件清单

| 资源 | 位置 | 格式 |
|------|------|------|
| 主程序图标 | `res/2015Remote.ico` | ICO |
| 托盘图标 | `res/tray_*.ico` | ICO |
| 工具栏图标 | `res/toolbar.bmp` | BMP |

### C. 常见问题

| 问题 | 原因 | 解决方案 |
|------|------|---------|
| 编译报错 | VS 版本/工具集 | 确认使用 VS2019+ 和 v142 工具集 |
| 中文乱码 | 文件编码错误 | UIBranding.h 需 UTF-8 with BOM |
| 图标不显示 | 格式不正确 | 确认 ICO 包含多尺寸 |
| 翻译不生效 | 编码错误 | 语言文件需 GB2312 编码 |
| 菜单仍显示 | 未重编译 | 修改 UIBranding.h 后需重编译 |

### D. 注意事项

**请勿修改以下配置**（会导致功能异常）：

```cpp
// 修改会导致所有已发放许可证失效
#define BRAND_LICENSE_MAGIC     "YAMA"

// 修改会导致单实例检测失败
#define BRAND_EVENT_PREFIX      "YAMA"

// 修改会导致超管密码配置失效
#define BRAND_ENV_VAR           "YAMA_PWD"
```

---

## 技术支持

| 渠道 | 联系方式 |
|------|---------|
| QQ | 962914132 |
| Telegram | [@doge_grandfather](https://t.me/doge_grandfather) |

---

## 相关文档

- [快速部署指南](QuickStart.md) - 基础部署
- [多级网络搭建指南](NetworkSetup.md) - 网络架构
- [日常使用手册](UserManual.md) - 功能使用
- [代理商运营手册](AgentManual.md) - 授权管理
