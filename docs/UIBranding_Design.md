# UI 定制化指南（编译时贴牌）

本指南面向下级开发商，说明如何定制程序品牌。

## 一、定制能力

- 修改程序品牌名称（窗口标题、启动画面、托盘提示、关于对话框）
- 隐藏不需要的菜单项（主菜单和右键菜单）
- 隐藏不需要的工具栏按钮
- 替换图标资源

所有配置在编译时固化，运行时用户无法修改。

## 二、环境要求

- **编译器**：Visual Studio 2019 或更高版本
- **项目文件**：`server/2015Remote/2015Remote.vcxproj`

## 三、定制流程

```
1. 用 Visual Studio 打开项目
2. 编辑 UIBranding.h 修改品牌名称和隐藏选项
3. 替换 res/ 目录下的图标资源（可选）
4. 编译项目（Build → Build Solution）
5. 在 Bin/ 目录获取编译结果
```

## 四、品牌名称配置

编辑 `UIBranding.h`（位于 `server/2015Remote/` 目录）：

```cpp
// 程序显示名称（窗口标题、关于对话框、系统菜单）
#define BRAND_APP_NAME          "MyRemote"

// 启动画面显示名称（通常为大写 logo 风格）
#define BRAND_SPLASH_NAME       "MYREMOTE"

// 托盘图标提示文本
#define BRAND_TRAY_TIP          "MyRemote 远程管理"

// 版权信息（关于对话框显示）
#define BRAND_COPYRIGHT         "Copyright (C) 2024 MyCompany"

// 服务显示名称（services.msc 中显示）
#define BRAND_SERVICE_DISPLAY   "MyRemote Control Service"

// 崩溃转储文件前缀（生成 前缀_日期时间.dmp）
#define BRAND_DUMP_PREFIX       "MYREMOTE"

// 默认可执行文件名（文件对话框建议名）
#define BRAND_EXE_NAME          "MYREMOTE.exe"

// 许可证文件类型描述（文件对话框过滤器）
#define BRAND_LICENSE_DESC      "MyRemote License"

// 许可证文件名前缀（导出时 前缀_设备ID.lic）
#define BRAND_LICENSE_PREFIX    "MYREMOTE"
```

> **提示**：只需修改引号内的文本，保持其他格式不变。

## 五、隐藏菜单项

在 `UIBranding.h` 中找到对应的宏，将值从 `0` 改为 `1` 即可隐藏。

```cpp
// 0 = 显示（默认）
// 1 = 隐藏

#define HIDE_MENU_WALLET                1   // 将钱包菜单隐藏
```

### 5.1 主菜单

| 菜单 | 可隐藏项 |
|------|----------|
| 文件 | 参数设置、提醒设置、钱包、网络 |
| 工具 | 输入密码、导入许可证、PE资源编辑、生成授权、生成Master、许可证管理、V2私钥 |
| 工具→ShellCode | C Code、bin、Load Test、混淆、混淆bin、混淆Load Test、AES C、AES bin、AES Load Test |
| 参数 | 键盘记录、登录通知、启用日志、壁纸隐私、V2文件协议、钩子窗口、作为服务运行、以用户身份运行 |
| 扩展 | 历史客户端、备份数据、导入数据、重新加载插件、插件请求、下级FRP |
| 扩展→语言 | 更改语言、选择语言目录 |
| 扩展→IP定位 | QQWry定位、Ip2Region定位 |
| 帮助 | 重要说明、反馈、什么是这个、主控跟踪、请求授权 |

### 5.2 右键菜单

| 分类 | 可隐藏项 |
|------|----------|
| 基本操作 | 发送消息、更新、删除、分享、主机备注、重新分组 |
| 远程桌面 | 虚拟桌面、灰度桌面、远程桌面、H264桌面 |
| 授权管理 | 授权、取消授权、分配给 |
| 监控功能 | 添加监视、登录通知、私有屏幕 |
| 系统操作 | 以管理员运行、卸载、代理、代理端口、标准代理端口、注入记事本 |
| 机器管理 | 关机、重启、注销 |
| 执行命令 | 下载执行、上传执行、测试运行、Ghost执行 |

完整宏名称见 `UIBranding.h` 文件中的注释。

## 六、隐藏工具栏按钮

```cpp
#define HIDE_TOOLBAR_TERMINAL           0   // 终端管理
#define HIDE_TOOLBAR_PROCESS            0   // 进程管理
#define HIDE_TOOLBAR_WINDOW             0   // 窗口管理
#define HIDE_TOOLBAR_DESKTOP            0   // 桌面管理
#define HIDE_TOOLBAR_FILE               0   // 文件管理
#define HIDE_TOOLBAR_AUDIO              0   // 语音管理
#define HIDE_TOOLBAR_VIDEO              0   // 视频管理
#define HIDE_TOOLBAR_SERVICE            0   // 服务管理
#define HIDE_TOOLBAR_REGISTER           0   // 注册表管理
#define HIDE_TOOLBAR_KEYBOARD           0   // 键盘记录
#define HIDE_TOOLBAR_SETTINGS           0   // 参数设置
#define HIDE_TOOLBAR_BUILD              0   // 生成服务端
#define HIDE_TOOLBAR_SEARCH             0   // 搜索
#define HIDE_TOOLBAR_HELP               0   // 帮助
```

## 七、图标定制

图标资源位于 `res/` 目录，完整列表见 `res/README.md`。

### 7.1 常用图标

| 文件 | 用途 | 规格 |
|------|------|------|
| `2015Remote.ico` | 主程序图标 | 多尺寸 ICO |
| `cmdshell.ico` | 远程终端窗口 | ICO |
| `screen.ico` | 远程桌面窗口 | ICO |
| `ToolBar_Main.bmp` | 主工具栏 | 48×48 多图拼接 |
| `Bitmap/*.bmp` | 菜单图标 | 16×16 BMP |

### 7.2 替换步骤

1. 准备相同规格的图标文件
2. 替换对应文件（保持文件名不变）
3. 重新编译

> **注意**：工具栏位图为多图横向拼接，替换时保持图标数量和顺序不变。

## 八、定制示例

以下示例展示如何创建一个简化版本，隐藏高级功能：

```cpp
// ===== 只需修改以下内容，其他保持默认 =====

// 品牌名称
#define BRAND_APP_NAME          "MyRemote"
#define BRAND_SPLASH_NAME       "MYREMOTE"
#define BRAND_TRAY_TIP          "MyRemote 远程管理"
#define BRAND_COPYRIGHT         "Copyright (C) 2024 MyCompany"

// 隐藏敏感功能（改为 1）
#define HIDE_MENU_WALLET                1
#define HIDE_MENU_GEN_AUTH              1
#define HIDE_MENU_GEN_MASTER            1
#define HIDE_MENU_LICENSE_MGR           1
#define HIDE_MENU_V2_PRIVATEKEY         1

// 隐藏工具栏按钮
#define HIDE_TOOLBAR_KEYBOARD           1
#define HIDE_TOOLBAR_REGISTER           1

// 隐藏右键菜单项
#define HIDE_CTX_AUTHORIZE              1
#define HIDE_CTX_UNAUTHORIZE            1
```

## 九、注意事项

### 9.1 编码要求

`UIBranding.h` 如果包含中文，必须保存为 **UTF-8 with BOM** 编码。

### 9.2 子菜单自动清理

当子菜单所有项目都被隐藏时，父级菜单会自动消失。

### 9.3 多语言支持

修改品牌名称后，如需多语言支持，请在 `lang/*.ini` 中添加对应翻译：

```ini
; lang/en_US.ini
MyRemote=MyRemote
MyRemote 远程管理=MyRemote Remote Management
```

### 9.4 运行时状态

部分菜单项有勾选状态（如"键盘记录"），隐藏后状态显示不可用，但不影响程序运行。
