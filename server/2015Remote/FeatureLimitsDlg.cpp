#include "stdafx.h"
#include "FeatureLimitsDlg.h"
#include "UIBranding.h"

CFeatureLimitsDlg::CFeatureLimitsDlg(CWnd* pParent)
    : CDialogLangEx(IDD_FEATURE_LIMITS, pParent)
    , m_InheritedMenuFlags(0)
    , m_InheritedToolbarFlags(0)
    , m_InheritedContextFlags(0)
    , m_MenuFlags(0)
    , m_ToolbarFlags(0)
    , m_ContextFlags(0)
    , m_CurrentTab(0)
{
    InitMenuItems();
    InitToolbarItems();
    InitContextItems();
}

CFeatureLimitsDlg::~CFeatureLimitsDlg()
{
}

void CFeatureLimitsDlg::SetInheritedFlags(const FeatureFlags* inherited)
{
    if (inherited) {
        m_InheritedMenuFlags = inherited->MenuFlags;
        m_InheritedToolbarFlags = inherited->ToolbarFlags;
        m_InheritedContextFlags = inherited->ContextFlags;
    }
}

void CFeatureLimitsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogLangEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_FEATURES, m_TabCtrl);
    DDX_Control(pDX, IDC_LIST_FEATURES, m_CheckList);
}

BEGIN_MESSAGE_MAP(CFeatureLimitsDlg, CDialogLangEx)
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_FEATURES, &CFeatureLimitsDlg::OnTcnSelchangeTab)
END_MESSAGE_MAP()

BOOL CFeatureLimitsDlg::OnInitDialog()
{
    CDialogLangEx::OnInitDialog();

    // 设置翻译文本
    SetWindowText(_TR("下级功能限制"));
    SetDlgItemText(IDC_STATIC_FEATURE_TIP, _TR("勾选: 对下级隐藏   灰色: 上级已禁用"));
    SetDlgItemText(IDOK, _TR("确定"));
    SetDlgItemText(IDCANCEL, _TR("取消"));

    // 添加Tab页
    m_TabCtrl.InsertItem(0, _TR("主菜单"));
    m_TabCtrl.InsertItem(1, _TR("工具栏"));
    m_TabCtrl.InsertItem(2, _TR("右键菜单"));

    // 初始显示第一页
    m_CurrentTab = 0;
    PopulateList(0);

    return TRUE;
}

void CFeatureLimitsDlg::InitMenuItems()
{
    m_MenuItems = {
        // 文件菜单
        { "参数设置", MF_SETTINGS, HIDE_MENU_SETTINGS },
        { "提醒设置", MF_NOTIFY_SETTINGS, HIDE_MENU_NOTIFY_SETTINGS },
        { "钱包", MF_WALLET, HIDE_MENU_WALLET },
        { "网络", MF_NETWORK, HIDE_MENU_NETWORK },
        // 工具菜单
        { "输入密码", MF_INPUT_PASSWORD, HIDE_MENU_INPUT_PASSWORD },
        { "导入许可证", MF_IMPORT_LICENSE, HIDE_MENU_IMPORT_LICENSE },
        { "PE资源编辑", MF_RCEDIT, HIDE_MENU_RCEDIT },
        { "生成授权", MF_GEN_AUTH, HIDE_MENU_GEN_AUTH },
        { "生成Master", MF_GEN_MASTER, HIDE_MENU_GEN_MASTER },
        { "许可证管理", MF_LICENSE_MGR, HIDE_MENU_LICENSE_MGR },
        { "V2私钥", MF_V2_PRIVATEKEY, HIDE_MENU_V2_PRIVATEKEY },
        // ShellCode子菜单
        { "ShellCode C Code", MF_SHELLCODE_C, HIDE_MENU_SHELLCODE_C },
        { "ShellCode bin", MF_SHELLCODE_BIN, HIDE_MENU_SHELLCODE_BIN },
        { "ShellCode Load Test", MF_SHELLCODE_LOAD_TEST, HIDE_MENU_SHELLCODE_LOAD_TEST },
        { "混淆 ShellCode", MF_SHELLCODE_OBFS, HIDE_MENU_SHELLCODE_OBFS },
        { "混淆 ShellCode bin", MF_SHELLCODE_OBFS_BIN, HIDE_MENU_SHELLCODE_OBFS_BIN },
        { "混淆 Load Test", MF_SHELLCODE_OBFS_TEST, HIDE_MENU_SHELLCODE_OBFS_TEST },
        { "AES ShellCode C", MF_SHELLCODE_AES_C, HIDE_MENU_SHELLCODE_AES_C },
        { "AES ShellCode bin", MF_SHELLCODE_AES_BIN, HIDE_MENU_SHELLCODE_AES_BIN },
        { "AES Load Test", MF_SHELLCODE_AES_TEST, HIDE_MENU_SHELLCODE_AES_TEST },
        // 参数菜单
        { "键盘记录", MF_KBLOGGER, HIDE_MENU_KBLOGGER },
        { "登录通知", MF_LOGIN_NOTIFY, HIDE_MENU_LOGIN_NOTIFY },
        { "启用日志", MF_ENABLE_LOG, HIDE_MENU_ENABLE_LOG },
        { "壁纸隐私", MF_PRIVACY_WALLPAPER, HIDE_MENU_PRIVACY_WALLPAPER },
        { "V2文件协议", MF_FILE_V2, HIDE_MENU_FILE_V2 },
        { "钩子窗口", MF_HOOK_WIN, HIDE_MENU_HOOK_WIN },
        { "作为服务运行", MF_RUN_AS_SERVICE, HIDE_MENU_RUN_AS_SERVICE },
        { "以用户身份运行", MF_RUN_AS_USER, HIDE_MENU_RUN_AS_USER },
        // 扩展菜单
        { "历史客户端", MF_HISTORY_CLIENTS, HIDE_MENU_HISTORY_CLIENTS },
        { "备份数据", MF_BACKUP_DATA, HIDE_MENU_BACKUP_DATA },
        { "导入数据", MF_IMPORT_DATA, HIDE_MENU_IMPORT_DATA },
        { "重新加载插件", MF_RELOAD_PLUGINS, HIDE_MENU_RELOAD_PLUGINS },
        { "插件请求", MF_PLUGIN_REQUEST, HIDE_MENU_PLUGIN_REQUEST },
        { "下级FRP", MF_FRPS_FOR_SUB, HIDE_MENU_FRPS_FOR_SUB },
        { "更改语言", MF_CHANGE_LANG, HIDE_MENU_CHANGE_LANG },
        { "选择语言目录", MF_CHOOSE_LANG_DIR, HIDE_MENU_CHOOSE_LANG_DIR },
        { "QQWry定位", MF_LOCATION_QQWRY, HIDE_MENU_LOCATION_QQWRY },
        { "Ip2Region定位", MF_LOCATION_IP2REGION, HIDE_MENU_LOCATION_IP2REGION },
        // 帮助菜单
        { "重要说明", MF_IMPORTANT, HIDE_MENU_IMPORTANT },
        { "反馈", MF_FEEDBACK, HIDE_MENU_FEEDBACK },
        { "什么是这个", MF_WHAT_IS_THIS, HIDE_MENU_WHAT_IS_THIS },
        { "主控跟踪", MF_MASTER_TRAIL, HIDE_MENU_MASTER_TRAIL },
        { "请求授权", MF_REQUEST_AUTH, HIDE_MENU_REQUEST_AUTH },
    };
}

void CFeatureLimitsDlg::InitToolbarItems()
{
    m_ToolbarItems = {
        { "终端管理", TF_TERMINAL, HIDE_TOOLBAR_TERMINAL },
        { "进程管理", TF_PROCESS, HIDE_TOOLBAR_PROCESS },
        { "窗口管理", TF_WINDOW, HIDE_TOOLBAR_WINDOW },
        { "桌面管理", TF_DESKTOP, HIDE_TOOLBAR_DESKTOP },
        { "文件管理", TF_FILE, HIDE_TOOLBAR_FILE },
        { "语音管理", TF_AUDIO, HIDE_TOOLBAR_AUDIO },
        { "视频管理", TF_VIDEO, HIDE_TOOLBAR_VIDEO },
        { "服务管理", TF_SERVICE, HIDE_TOOLBAR_SERVICE },
        { "注册表管理", TF_REGISTER, HIDE_TOOLBAR_REGISTER },
        { "键盘记录", TF_KEYBOARD, HIDE_TOOLBAR_KEYBOARD },
        { "参数设置", TF_SETTINGS, HIDE_TOOLBAR_SETTINGS },
        { "生成服务端", TF_BUILD, HIDE_TOOLBAR_BUILD },
        { "搜索", TF_SEARCH, HIDE_TOOLBAR_SEARCH },
        { "帮助", TF_HELP, HIDE_TOOLBAR_HELP },
    };
}

void CFeatureLimitsDlg::InitContextItems()
{
    m_ContextItems = {
        // 在线列表右键菜单
        { "发送消息", CF_MESSAGE, HIDE_CTX_MESSAGE },
        { "更新客户端", CF_UPDATE, HIDE_CTX_UPDATE },
        { "删除", CF_DELETE, HIDE_CTX_DELETE },
        { "分享", CF_SHARE, HIDE_CTX_SHARE },
        { "代理", CF_PROXY, HIDE_CTX_PROXY },
        { "主机备注", CF_HOSTNOTE, HIDE_CTX_HOSTNOTE },
        { "虚拟桌面", CF_VIRTUAL_DESKTOP, HIDE_CTX_VIRTUAL_DESKTOP },
        { "灰度桌面", CF_GRAY_DESKTOP, HIDE_CTX_GRAY_DESKTOP },
        { "远程桌面", CF_REMOTE_DESKTOP, HIDE_CTX_REMOTE_DESKTOP },
        { "H264桌面", CF_H264_DESKTOP, HIDE_CTX_H264_DESKTOP },
        { "授权", CF_AUTHORIZE, HIDE_CTX_AUTHORIZE },
        { "取消授权", CF_UNAUTHORIZE, HIDE_CTX_UNAUTHORIZE },
        { "分配给", CF_ASSIGN_TO, HIDE_CTX_ASSIGN_TO },
        { "添加监视", CF_ADD_WATCH, HIDE_CTX_ADD_WATCH },
        { "登录通知", CF_LOGIN_NOTIFY, HIDE_CTX_LOGIN_NOTIFY },
        { "以管理员运行", CF_RUN_AS_ADMIN, HIDE_CTX_RUN_AS_ADMIN },
        { "卸载", CF_UNINSTALL, HIDE_CTX_UNINSTALL },
        { "私有屏幕", CF_PRIVATE_SCREEN, HIDE_CTX_PRIVATE_SCREEN },
        { "重新分组", CF_REGROUP, HIDE_CTX_REGROUP },
        { "注入记事本", CF_INJ_NOTEPAD, HIDE_CTX_INJ_NOTEPAD },
        { "代理端口", CF_PROXY_PORT, HIDE_CTX_PROXY_PORT },
        { "标准代理端口", CF_PROXY_PORT_STD, HIDE_CTX_PROXY_PORT_STD },
        // 机器管理子菜单
        { "关机", CF_MACHINE_SHUTDOWN, HIDE_CTX_MACHINE_SHUTDOWN },
        { "重启", CF_MACHINE_REBOOT, HIDE_CTX_MACHINE_REBOOT },
        { "注销", CF_MACHINE_LOGOUT, HIDE_CTX_MACHINE_LOGOUT },
        // 执行命令子菜单
        { "下载执行", CF_EXECUTE_DOWNLOAD, HIDE_CTX_EXECUTE_DOWNLOAD },
        { "上传执行", CF_EXECUTE_UPLOAD, HIDE_CTX_EXECUTE_UPLOAD },
        { "测试运行", CF_EXECUTE_TESTRUN, HIDE_CTX_EXECUTE_TESTRUN },
        { "Ghost执行", CF_EXECUTE_GHOST, HIDE_CTX_EXECUTE_GHOST },
    };
}

void CFeatureLimitsDlg::PopulateList(int tabIndex)
{
    // 先收集当前Tab的选择
    CollectFlags();

    // 清空列表
    m_CheckList.ResetContent();

    // 根据Tab选择不同的项目列表
    std::vector<FeatureItem>* items = nullptr;
    uint64_t inheritedFlags = 0;
    uint64_t selectedFlags = 0;

    switch (tabIndex) {
    case 0:
        items = &m_MenuItems;
        inheritedFlags = m_InheritedMenuFlags;
        selectedFlags = m_MenuFlags;
        break;
    case 1:
        items = &m_ToolbarItems;
        inheritedFlags = m_InheritedToolbarFlags;
        selectedFlags = m_ToolbarFlags;
        break;
    case 2:
        items = &m_ContextItems;
        inheritedFlags = m_InheritedContextFlags;
        selectedFlags = m_ContextFlags;
        break;
    default:
        return;
    }

    // 添加项目到列表
    for (const auto& item : *items) {
        // 编译时已隐藏的功能，不显示
        if (item.compileMacro)
            continue;

        int index = m_CheckList.AddString(_TR(item.name));
        m_CheckList.SetItemData(index, item.flag);

        // 检查是否被上级禁用（继承）或用户已选择
        bool isInherited = (inheritedFlags & item.flag) != 0;
        bool isSelected = (selectedFlags & item.flag) != 0;

        if (isInherited || isSelected) {
            m_CheckList.SetCheck(index, TRUE);
        }

        // 上级已禁用的功能，禁用复选框（灰色，不可取消）
        if (isInherited) {
            m_CheckList.Enable(index, FALSE);
        }
    }

    m_CurrentTab = tabIndex;
}

void CFeatureLimitsDlg::CollectFlags()
{
    // 根据当前Tab收集选中状态到对应的标志位
    uint64_t* targetFlags = nullptr;
    uint64_t inheritedFlags = 0;

    switch (m_CurrentTab) {
    case 0:
        targetFlags = &m_MenuFlags;
        inheritedFlags = m_InheritedMenuFlags;
        break;
    case 1:
        targetFlags = &m_ToolbarFlags;
        inheritedFlags = m_InheritedToolbarFlags;
        break;
    case 2:
        targetFlags = &m_ContextFlags;
        inheritedFlags = m_InheritedContextFlags;
        break;
    default:
        return;
    }

    // 保留继承的标志位
    *targetFlags = inheritedFlags;

    // 收集用户勾选的项目
    for (int i = 0; i < m_CheckList.GetCount(); i++) {
        if (m_CheckList.GetCheck(i)) {
            *targetFlags |= m_CheckList.GetItemData(i);
        }
    }
}

void CFeatureLimitsDlg::OnTcnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult)
{
    int sel = m_TabCtrl.GetCurSel();
    PopulateList(sel);
    *pResult = 0;
}

void CFeatureLimitsDlg::OnOK()
{
    // 收集最后一个Tab的选择
    CollectFlags();

    CDialogLangEx::OnOK();
}
