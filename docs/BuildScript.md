# SimpleRemoter 编译脚本

命令行编译脚本，支持完整编译和增量编译。

## 环境要求

- Visual Studio 2019 或 2022
- v142 工具集 (MSVC v142 - VS 2019 C++ build tools)
- MFC 库 (C++ MFC for v142 build tools)

脚本自动检测以下 VS 安装路径：
- `C:\Program Files\Microsoft Visual Studio\18\*` (VS2019 Insiders)
- `C:\Program Files*\Microsoft Visual Studio\2019\*`
- `C:\Program Files*\Microsoft Visual Studio\2022\*`

## 脚本文件

| 文件 | 说明 |
|------|------|
| `build.ps1` | PowerShell 主脚本 |
| `build.cmd` | 批处理快捷入口 |

## 命令参数

### PowerShell (`build.ps1`)

```powershell
.\build.ps1 [-Config <配置>] [-Platform <平台>] [-ServerOnly] [-Clean] [-Publish]
```

| 参数 | 可选值 | 默认值 | 说明 |
|------|--------|--------|------|
| `-Config` | `Release`, `Debug` | `Release` | 编译配置 |
| `-Platform` | `x64`, `x86`, `all` | `x64` | 目标平台 |
| `-ServerOnly` | 开关 | 否 | 仅编译主控程序，跳过依赖项 |
| `-Clean` | 开关 | 否 | 清理后重新编译 |
| `-Publish` | 开关 | 否 | 发版模式：重编译所有 + UPX 压缩 |

### 批处理 (`build.cmd`)

```cmd
build [release|debug] [x64|x86|all] [server|clean|publish]
```

- 参数顺序任意
- 不区分大小写

## 使用示例

### PowerShell

```powershell
# 完整编译 (Release x64)
.\build.ps1

# 编译 x86 + x64 两个平台
.\build.ps1 -Platform all

# Debug 模式编译
.\build.ps1 -Config Debug

# 仅编译主控程序 (跳过依赖项，适合快速重编译)
.\build.ps1 -ServerOnly

# 清理后重新编译
.\build.ps1 -Clean

# 组合使用
.\build.ps1 -Config Debug -Platform all -Clean
```

### 批处理

```cmd
build                      # 完整编译 Release x64
build server               # 仅主控程序
build debug x86            # Debug x86
build release all clean    # 清理后编译所有平台
build publish              # 发版模式
```

## 编译流程

### 完整编译 (默认)

```
Step 1: 编译客户端项目 (始终 Release, x86 + x64)
   ├── ServerDll.dll    ← ClientDll_vs2015.vcxproj
   ├── ghost.exe        ← ghost_vs2015.vcxproj
   ├── TestRun.exe      ← TestRun_vs2015.vcxproj
   ├── TinyRun.dll      ← TinyRun.vcxproj
   └── SCLoader.exe     ← SCLoader.vcxproj

Step 2: 编译主控程序 (使用 -Config 指定的配置)
   └── Yama.exe         ← 2015Remote_vs2015.vcxproj
```

**注意：** 客户端项目始终使用 Release 模式编译，因为主控程序资源中引用的是 `Release\` 目录下的文件。`-Config Debug` 只影响主控程序本身。

主控程序 Yama 会将 Step 1 编译的 DLL/EXE 内嵌到资源中，因此**首次编译必须执行完整编译**。

### 发版模式 (-Publish)

```
Step 1: 重新编译所有客户端项目 (Release x86 + x64)
Step 2: 重新编译主控程序 (Release x64)
Step 3: UPX 压缩 (upx --best Yama_x64.exe)
```

发版模式会：
- 强制 Release x64 配置
- 强制 Clean 重新编译（确保所有文件是最新的）
- 使用 UPX 压缩最终输出（约 70% 压缩率）

UPX 路径：`server/2015Remote/res/3rd/upx.exe`

### 仅主控 (-ServerOnly)

跳过 Step 1，直接编译 Yama。适用于：
- 依赖项已编译完成
- 仅修改了主控程序代码

## 输出文件

| 平台 | 输出路径 |
|------|---------|
| x64 | `Bin\Yama_x64.exe` |
| x86 | `Bin\Yama_x86.exe` |

客户端项目输出：

| 项目 | x86 输出 | x64 输出 |
|------|---------|---------|
| ServerDll | `Release\ServerDll.dll` | `x64\Release\ServerDll.dll` |
| ghost | `Release\ghost.exe` | `x64\Release\ghost.exe` |
| TestRun | `Release\TestRun.exe` | `x64\Release\TestRun.exe` |
| TinyRun | `Release\TinyRun.dll` | `x64\Release\TinyRun.dll` |
| SCLoader | `Release\SCLoader.exe` | `x64\Release\SCLoader.exe` |

## 常见问题

### v142 build tools cannot be found

安装 VS2019 构建工具：
1. 打开 Visual Studio Installer
2. 修改 VS2022 或安装 VS2019
3. 勾选 `MSVC v142 - VS 2019 C++ x64/x86 build tools`

### MFC libraries are required

安装 MFC 库：
1. 打开 Visual Studio Installer
2. 勾选 `C++ MFC for v142 build tools (x86 and x64)`

### MSBuild not found

确保安装了 Visual Studio 2019 或 2022，且包含 C++ 桌面开发工作负载。
