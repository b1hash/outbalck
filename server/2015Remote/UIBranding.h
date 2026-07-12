#pragma once

// ============================================================
//  UIBranding.h - 品牌定制配置文件
// ============================================================
//
//  下级开发商通过修改此文件来定制程序品牌。
//  修改后重新编译即可生成定制版本。
//
//  编码要求：此文件必须保存为 UTF-8 with BOM（MSVC 要求）
//  注意：此文件中的配置会编译到程序中，运行时无法修改。
//
// ============================================================
//
//  快速定制：最少只需修改以下 4 项
//
//    BRAND_APP_NAME      - 程序名称（窗口标题）
//    BRAND_SPLASH_NAME   - 启动画面 Logo
//    BRAND_TRAY_TIP      - 托盘图标提示
//    BRAND_COPYRIGHT     - 版权信息
//
//  其他项（含版本号）可保持默认，或按需修改。
//
// ============================================================


// ============================================================
//  1. 品牌名称设置
// ============================================================
//
//  标注说明：
//    无标注    = 可使用中文和特殊字符
//    [建议XX]  = 风格建议，非强制
//    [仅ASCII] = 技术限制，只能用英文字母、数字、下划线、连字符
//
// ============================================================

// --- 1.1 界面显示（无限制，可用中文） ---

// 程序显示名称
// 影响：窗口标题、关于对话框
// 示例: "MyRemote" 或 "我的远程"
#define BRAND_APP_NAME          "Yama"

// 程序版本号 [建议格式: X.Y.Z]
// 影响：关于对话框、标题栏
#define BRAND_VERSION           "1.3.1"

// 启动画面名称 [建议大写，更有 Logo 感]
// 影响：启动画面 Logo 文字（大号艺术字体渲染）
// 示例: "MYREMOTE" 或 "我的远程"
#define BRAND_SPLASH_NAME       "YAMA"

// 托盘图标提示
// 影响：鼠标悬停在托盘图标时显示的文字
// 示例: "MyRemote 远程管理"
#define BRAND_TRAY_TIP          "禁界: 远程协助软件"

// 版权信息
// 影响：关于对话框
// 示例: "Copyright (C) 2024 MyCompany"
#define BRAND_COPYRIGHT         "Copyleft (C) 2019-2026"

// 服务显示名称
// 影响：services.msc 中的显示名称
// 示例: "MyRemote Control Service" 或 "我的远程服务"
#define BRAND_SERVICE_DISPLAY   "Yama Control Service"

// 许可证文件描述
// 影响：文件对话框过滤器显示
// 示例: "MyRemote 许可证"
#define BRAND_LICENSE_DESC      "YAMA License"

// --- 1.2 文件和目录 [仅ASCII] ---
//
// 技术限制：文件名/目录名建议使用 ASCII 字符，避免跨系统兼容问题
// 禁止字符：\ / : * ? " < > |

// 崩溃转储前缀 [仅ASCII]
// 生成文件：{前缀}_YYYY-MM-DD HHMMSS.dmp
#define BRAND_DUMP_PREFIX       "YAMA"

// 默认 EXE 文件名 [仅ASCII]
// 影响：生成服务端时的默认文件名
#define BRAND_EXE_NAME          "YAMA.exe"

// 许可证文件前缀 [仅ASCII]
// 生成文件：{前缀}_{设备ID}.lic
#define BRAND_LICENSE_PREFIX    "YAMA"

// 数据库文件名 [仅ASCII]
// 存储位置：%APPDATA%\{BRAND_DATA_FOLDER}\{此文件名}
#define BRAND_DB_NAME           "YAMA.db"

// 数据目录名 [仅ASCII]
// 存储位置：%APPDATA%\{此目录名}
#define BRAND_DATA_FOLDER       "YAMA"

// --- 1.3 系统标识 [仅ASCII，无空格] ---
//
// 技术限制：Windows 服务名、注册表键名必须是 ASCII，且不能有空格

// Windows 服务名称 [仅ASCII，无空格]
// 影响：注册表路径、net start/stop 命令
// 命令: net stop YamaControlService
#define BRAND_SERVICE_NAME      "YamaControlService"

// 注册表键名 [仅ASCII，无空格]
// 存储位置：HKCU\Software\{此键名}
#define BRAND_REGISTRY_KEY      "YAMA"

// 网络通信前缀 [仅ASCII，无空格]
// 用于：内部 TCP/UDP/文件服务器标识
#define BRAND_NET_PREFIX        "YAMA"


// ============================================================
//  2. 隐藏主菜单项
// ============================================================
//
//  使用方法：
//    0 = 显示（默认）
//    1 = 隐藏
//
// ============================================================

// --- 文件菜单 ---
#define HIDE_MENU_SETTINGS              0   // 参数设置
#define HIDE_MENU_NOTIFY_SETTINGS       0   // 提醒设置
#define HIDE_MENU_WALLET                0   // 钱包
#define HIDE_MENU_NETWORK               0   // 网络
// 注意：退出菜单不可隐藏

// --- 工具菜单 ---
#define HIDE_MENU_INPUT_PASSWORD        0   // 输入密码
#define HIDE_MENU_IMPORT_LICENSE        0   // 导入许可证
#define HIDE_MENU_RCEDIT                0   // PE资源编辑
#define HIDE_MENU_GEN_AUTH              0   // 生成授权
#define HIDE_MENU_GEN_MASTER            0   // 生成Master
#define HIDE_MENU_LICENSE_MGR           0   // 许可证管理
#define HIDE_MENU_V2_PRIVATEKEY         0   // V2私钥

// --- 工具菜单 - ShellCode子菜单 ---
#define HIDE_MENU_SHELLCODE_C           0   // C Code
#define HIDE_MENU_SHELLCODE_BIN         0   // bin
#define HIDE_MENU_SHELLCODE_LOAD_TEST   0   // Load Test
#define HIDE_MENU_SHELLCODE_OBFS        0   // 混淆
#define HIDE_MENU_SHELLCODE_OBFS_BIN    0   // 混淆 bin
#define HIDE_MENU_SHELLCODE_OBFS_TEST   0   // 混淆 Load Test
#define HIDE_MENU_SHELLCODE_AES_C       0   // AES C
#define HIDE_MENU_SHELLCODE_AES_BIN     0   // AES bin
#define HIDE_MENU_SHELLCODE_AES_TEST    0   // AES Load Test

// --- 参数菜单 ---
#define HIDE_MENU_KBLOGGER              0   // 键盘记录
#define HIDE_MENU_LOGIN_NOTIFY          0   // 登录通知
#define HIDE_MENU_ENABLE_LOG            0   // 启用日志
#define HIDE_MENU_PRIVACY_WALLPAPER     0   // 壁纸隐私
#define HIDE_MENU_FILE_V2               0   // V2文件协议
#define HIDE_MENU_HOOK_WIN              0   // 钩子窗口
#define HIDE_MENU_RUN_AS_SERVICE        0   // 作为服务运行
#define HIDE_MENU_RUN_AS_USER           0   // 以用户身份运行

// --- 扩展菜单 ---
#define HIDE_MENU_HISTORY_CLIENTS       0   // 历史客户端
#define HIDE_MENU_BACKUP_DATA           0   // 备份数据
#define HIDE_MENU_IMPORT_DATA           0   // 导入数据
#define HIDE_MENU_RELOAD_PLUGINS        0   // 重新加载插件
#define HIDE_MENU_PLUGIN_REQUEST        0   // 插件请求
#define HIDE_MENU_FRPS_FOR_SUB          0   // 下级FRP

// --- 扩展菜单 - 语言子菜单 ---
#define HIDE_MENU_CHANGE_LANG           0   // 更改语言
#define HIDE_MENU_CHOOSE_LANG_DIR       0   // 选择语言目录

// --- 扩展菜单 - IP定位子菜单 ---
#define HIDE_MENU_LOCATION_QQWRY        0   // QQWry定位
#define HIDE_MENU_LOCATION_IP2REGION    0   // Ip2Region定位

// --- 帮助菜单 ---
#define HIDE_MENU_IMPORTANT             0   // 重要说明
#define HIDE_MENU_FEEDBACK              0   // 反馈
#define HIDE_MENU_WHAT_IS_THIS          0   // 什么是这个
#define HIDE_MENU_MASTER_TRAIL          0   // 主控跟踪
#define HIDE_MENU_REQUEST_AUTH          0   // 请求授权


// ============================================================
//  3. 隐藏工具栏按钮（0=显示，1=隐藏）
// ============================================================

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


// ============================================================
//  4. 隐藏右键菜单项（0=显示，1=隐藏）
// ============================================================

// --- 基本操作 ---
#define HIDE_CTX_MESSAGE                0   // 发送消息
#define HIDE_CTX_UPDATE                 0   // 更新客户端
#define HIDE_CTX_DELETE                 0   // 删除
#define HIDE_CTX_SHARE                  0   // 分享
#define HIDE_CTX_HOSTNOTE               0   // 主机备注
#define HIDE_CTX_REGROUP                0   // 重新分组

// --- 远程桌面 ---
#define HIDE_CTX_VIRTUAL_DESKTOP        0   // 虚拟桌面
#define HIDE_CTX_GRAY_DESKTOP           0   // 灰度桌面
#define HIDE_CTX_REMOTE_DESKTOP         0   // 远程桌面
#define HIDE_CTX_H264_DESKTOP           0   // H264桌面
#define HIDE_CTX_PRIVATE_SCREEN         0   // 私有屏幕

// --- 授权管理 ---
#define HIDE_CTX_AUTHORIZE              0   // 授权
#define HIDE_CTX_UNAUTHORIZE            0   // 取消授权
#define HIDE_CTX_ASSIGN_TO              0   // 分配给

// --- 监控功能 ---
#define HIDE_CTX_ADD_WATCH              0   // 添加监视
#define HIDE_CTX_LOGIN_NOTIFY           0   // 登录通知

// --- 系统操作 ---
#define HIDE_CTX_RUN_AS_ADMIN           0   // 以管理员运行
#define HIDE_CTX_UNINSTALL              0   // 卸载
#define HIDE_CTX_PROXY                  0   // 代理
#define HIDE_CTX_PROXY_PORT             0   // 代理端口
#define HIDE_CTX_PROXY_PORT_STD         0   // 标准代理端口
#define HIDE_CTX_INJ_NOTEPAD            0   // 注入记事本

// --- 机器管理子菜单 ---
#define HIDE_CTX_MACHINE_SHUTDOWN       0   // 关机
#define HIDE_CTX_MACHINE_REBOOT         0   // 重启
#define HIDE_CTX_MACHINE_LOGOUT         0   // 注销

// --- 执行命令子菜单 ---
#define HIDE_CTX_EXECUTE_DOWNLOAD       0   // 下载执行
#define HIDE_CTX_EXECUTE_UPLOAD         0   // 上传执行
#define HIDE_CTX_EXECUTE_TESTRUN        0   // 测试运行
#define HIDE_CTX_EXECUTE_GHOST          0   // Ghost执行


// ============================================================
//  5. 外部链接配置
// ============================================================
//
//  这些链接可通过配置文件覆盖，无需重新编译。
//  配置键名：settings/FeedbackUrl, settings/HelpUrl, settings/RequestAuthUrl
//
//  如果配置值以 "http" 开头，则作为 URL 打开浏览器。
//  否则直接显示为文本消息。
//

// 反馈链接（帮助菜单 → 反馈）
#define BRAND_URL_FEEDBACK      "https://t.me/SimpleRemoter"

// 帮助文档链接（帮助菜单 → 什么是这个）
#define BRAND_URL_WIKI          "https://github.com/yuanyuanxiang/SimpleRemoter/wiki"

// 请求授权链接（工具菜单 → 请求授权）
#define BRAND_URL_REQUEST_AUTH  "https://github.com/yuanyuanxiang/SimpleRemoter/wiki#请求授权"

// 获取插件
#define BRAND_URL_GET_PLUGIN "This feature has not been implemented!\nPlease contact: 962914132@qq.com"

// ============================================================
//  内部使用 - 请勿修改以下内容
// ============================================================

// --- 系统保留宏（修改会导致功能异常）---
//
// 警告：以下宏用于系统内部标识，修改会导致：
//   - BRAND_LICENSE_MAGIC: 所有已发放许可证失效
//   - BRAND_EVENT_PREFIX:  单实例检测失败
//   - BRAND_ENV_VAR:       超管密码配置失效
//
#define BRAND_LICENSE_MAGIC     "YAMA"      // 许可证魔数
#define BRAND_EVENT_PREFIX      "YAMA"      // 进程事件名前缀
#define BRAND_ENV_VAR           "YAMA_PWD"  // 环境变量名（set YAMA_PWD=密码）

// --- 宽字符版本（自动生成）---
#define BRAND_APP_NAME_W        _T(BRAND_APP_NAME)
#define BRAND_SPLASH_NAME_W     _T(BRAND_SPLASH_NAME)
#define BRAND_TRAY_TIP_W        _T(BRAND_TRAY_TIP)
#define BRAND_COPYRIGHT_W       _T(BRAND_COPYRIGHT)
#define BRAND_VERSION_W         _T(BRAND_VERSION)
#define BRAND_SERVICE_DISPLAY_W _T(BRAND_SERVICE_DISPLAY)
#define BRAND_EXE_NAME_W        _T(BRAND_EXE_NAME)
#define BRAND_LICENSE_DESC_W    _T(BRAND_LICENSE_DESC)
#define BRAND_LICENSE_PREFIX_W  _T(BRAND_LICENSE_PREFIX)
#define BRAND_DB_NAME_W         _T(BRAND_DB_NAME)
#define BRAND_DATA_FOLDER_W     _T(BRAND_DATA_FOLDER)
#define BRAND_SERVICE_NAME_W    _T(BRAND_SERVICE_NAME)
#define BRAND_REGISTRY_KEY_W    _T(BRAND_REGISTRY_KEY)
#define BRAND_NET_PREFIX_W      _T(BRAND_NET_PREFIX)
