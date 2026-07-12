#include "stdafx.h"
#include "FrpsForSubDlg.h"
#include "context.h"
#include "CPasswordDlg.h"
#include "2015Remote.h"
#include "2015RemoteDlg.h"
#include "resource.h"

// 获取 FRP 端口分配文件路径（与 licenses.ini 同目录）
std::string CFrpsForSubDlg::GetFrpPortsPath()
{
    std::string dbPath = GetDbPath();
    size_t pos = dbPath.find_last_of("\\/");
    if (pos != std::string::npos) {
        return dbPath.substr(0, pos + 1) + "frp_ports.ini";
    }
    return "frp_ports.ini";
}

CFrpsForSubDlg::CFrpsForSubDlg(CWnd* pParent)
    : CDialogLangEx(IDD_DIALOG_FRPS_FOR_SUB, pParent)
    , m_bShowToken(false)
{
}

CFrpsForSubDlg::~CFrpsForSubDlg()
{
}

void CFrpsForSubDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogLangEx::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_CHECK_FRPS_ENABLED, m_checkEnabled);
    DDX_Control(pDX, IDC_CHECK_FRPS_LOCAL, m_checkLocalFrps);
    DDX_Control(pDX, IDC_EDIT_FRPS_SERVER, m_editServer);
    DDX_Control(pDX, IDC_EDIT_FRPS_PORT, m_editPort);
    DDX_Control(pDX, IDC_EDIT_FRPS_TOKEN, m_editToken);
    DDX_Control(pDX, IDC_BTN_SHOW_TOKEN, m_btnShowToken);
    DDX_Control(pDX, IDC_EDIT_PORT_START, m_editPortStart);
    DDX_Control(pDX, IDC_EDIT_PORT_END, m_editPortEnd);
    DDX_Control(pDX, IDC_RADIO_FRP_OFFICIAL, m_radioOfficial);
    DDX_Control(pDX, IDC_RADIO_FRP_CUSTOM, m_radioCustom);
}

BEGIN_MESSAGE_MAP(CFrpsForSubDlg, CDialogLangEx)
    ON_BN_CLICKED(IDC_CHECK_FRPS_ENABLED, &CFrpsForSubDlg::OnBnClickedCheckFrpsEnabled)
    ON_BN_CLICKED(IDC_CHECK_FRPS_LOCAL, &CFrpsForSubDlg::OnBnClickedCheckFrpsLocal)
    ON_BN_CLICKED(IDC_BTN_SHOW_TOKEN, &CFrpsForSubDlg::OnBnClickedBtnShowToken)
END_MESSAGE_MAP()

BOOL CFrpsForSubDlg::OnInitDialog()
{
    CDialogLangEx::OnInitDialog();

    // 设置翻译文本
    SetWindowText(_TR("下级 FRP 代理设置"));
    SetDlgItemText(IDC_CHECK_FRPS_ENABLED, _TR("启用为下级提供 FRP 代理"));
    SetDlgItemText(IDC_CHECK_FRPS_LOCAL, _TR("FRPS 运行在本机"));
    SetDlgItemText(IDC_GROUP_FRPS_SERVER, _TR("FRPS 服务器配置"));

#ifdef _WIN64
    // 64 位程序启用本地 FRPS 选项
    m_checkLocalFrps.EnableWindow(TRUE);
#else
    // 32 位程序禁用本地 FRPS 选项（frps.dll 仅支持 64 位）
    m_checkLocalFrps.EnableWindow(FALSE);
    m_checkLocalFrps.SetCheck(BST_UNCHECKED);
#endif
    SetDlgItemText(IDC_STATIC_FRP_AUTHMODE, _TR("认证模式:"));
    SetDlgItemText(IDC_RADIO_FRP_OFFICIAL, _TR("官方 FRP"));
    SetDlgItemText(IDC_RADIO_FRP_CUSTOM, _TR("自定义 FRP"));
    SetDlgItemText(IDC_STATIC_FRPS_SERVER, _TR("服务器地址:"));
    SetDlgItemText(IDC_STATIC_FRPS_PORT, _TR("服务器端口:"));
    SetDlgItemText(IDC_STATIC_FRPS_TOKEN, _TR("认证Token:"));
    SetDlgItemText(IDC_GROUP_PORT_RANGE, _TR("端口分配范围（可选）"));
    SetDlgItemText(IDC_STATIC_PORT_START, _TR("起始端口:"));
    SetDlgItemText(IDC_STATIC_PORT_END, _TR("结束端口:"));
    SetDlgItemText(IDC_STATIC_FRPS_TIP, _TR("提示: 配置的 FRPS 服务器将用于为下级提供反向代理服务"));
    SetDlgItemText(IDOK, _TR("确定"));
    SetDlgItemText(IDCANCEL, _TR("取消"));

    // 设置 Token 显示/隐藏按钮文字
    m_btnShowToken.SetWindowText(_T("*"));

    LoadSettings();
    UpdateControlStates();

    return TRUE;
}

void CFrpsForSubDlg::LoadSettings()
{
    m_config = GetFrpsConfig();

    m_checkEnabled.SetCheck(m_config.enabled ? BST_CHECKED : BST_UNCHECKED);
#ifdef _WIN64
    m_checkLocalFrps.SetCheck(m_config.localFrps ? BST_CHECKED : BST_UNCHECKED);
#else
    m_checkLocalFrps.SetCheck(BST_UNCHECKED);
#endif
    m_editServer.SetWindowText(CString(m_config.server.c_str()));

    CString portStr;
    portStr.Format(_T("%d"), m_config.port);
    m_editPort.SetWindowText(portStr);

    m_editToken.SetWindowText(CString(m_config.token.c_str()));

    CString portStartStr, portEndStr;
    portStartStr.Format(_T("%d"), m_config.portStart);
    portEndStr.Format(_T("%d"), m_config.portEnd);
    m_editPortStart.SetWindowText(portStartStr);
    m_editPortEnd.SetWindowText(portEndStr);

    // 设置认证模式单选按钮
    m_radioOfficial.SetCheck(m_config.authMode == FRP_AUTH_TOKEN ? BST_CHECKED : BST_UNCHECKED);
    m_radioCustom.SetCheck(m_config.authMode == FRP_AUTH_PRIVILEGE_KEY ? BST_CHECKED : BST_UNCHECKED);
}

void CFrpsForSubDlg::SaveSettings()
{
    CString str;

    m_config.enabled = (m_checkEnabled.GetCheck() == BST_CHECKED);
#ifdef _WIN64
    m_config.localFrps = (m_checkLocalFrps.GetCheck() == BST_CHECKED);
#else
    m_config.localFrps = false;
#endif

    m_editServer.GetWindowText(str);
    m_config.server = CT2A(str, CP_UTF8);

    m_editPort.GetWindowText(str);
    m_config.port = _ttoi(str);

    m_editToken.GetWindowText(str);
    m_config.token = CT2A(str, CP_UTF8);

    m_editPortStart.GetWindowText(str);
    m_config.portStart = _ttoi(str);

    m_editPortEnd.GetWindowText(str);
    m_config.portEnd = _ttoi(str);

    // 获取认证模式
    m_config.authMode = (m_radioOfficial.GetCheck() == BST_CHECKED) ? FRP_AUTH_TOKEN : FRP_AUTH_PRIVILEGE_KEY;

    // 保存到 INI 文件
    THIS_CFG.SetInt("frps_for_sub", "enabled", m_config.enabled ? 1 : 0);
    THIS_CFG.SetInt("frps_for_sub", "local_frps", m_config.localFrps ? 1 : 0);
    THIS_CFG.SetStr("frps_for_sub", "server", m_config.server.c_str());
    THIS_CFG.SetInt("frps_for_sub", "port", m_config.port);
    THIS_CFG.SetStr("frps_for_sub", "token", m_config.token.c_str());
    THIS_CFG.SetInt("frps_for_sub", "port_start", m_config.portStart);
    THIS_CFG.SetInt("frps_for_sub", "port_end", m_config.portEnd);
    THIS_CFG.SetInt("frps_for_sub", "auth_mode", m_config.authMode);
}

void CFrpsForSubDlg::UpdateControlStates()
{
    BOOL enabled = (m_checkEnabled.GetCheck() == BST_CHECKED);
    BOOL localFrps = (m_checkLocalFrps.GetCheck() == BST_CHECKED);

#ifdef _WIN64
    m_checkLocalFrps.EnableWindow(enabled);
#else
    m_checkLocalFrps.EnableWindow(FALSE);
#endif

    // 如果启用本地 FRPS，服务器地址自动设为 127.0.0.1 且禁用编辑
    BOOL serverEditable = enabled && !localFrps;
    m_editServer.EnableWindow(serverEditable);
    if (localFrps && enabled) {
        m_editServer.SetWindowText(_T("127.0.0.1"));
    }

    m_editPort.EnableWindow(enabled);
    m_editToken.EnableWindow(enabled);
    m_btnShowToken.EnableWindow(enabled);
    m_editPortStart.EnableWindow(enabled);
    m_editPortEnd.EnableWindow(enabled);
    m_radioOfficial.EnableWindow(enabled);
    m_radioCustom.EnableWindow(enabled);

    // 更新分组框状态（视觉效果）
    GetDlgItem(IDC_GROUP_FRPS_SERVER)->EnableWindow(enabled);
    GetDlgItem(IDC_GROUP_PORT_RANGE)->EnableWindow(enabled);
    GetDlgItem(IDC_STATIC_FRP_AUTHMODE)->EnableWindow(enabled);
    GetDlgItem(IDC_STATIC_FRPS_SERVER)->EnableWindow(serverEditable);
    GetDlgItem(IDC_STATIC_FRPS_PORT)->EnableWindow(enabled);
    GetDlgItem(IDC_STATIC_FRPS_TOKEN)->EnableWindow(enabled);
    GetDlgItem(IDC_STATIC_PORT_START)->EnableWindow(enabled);
    GetDlgItem(IDC_STATIC_PORT_END)->EnableWindow(enabled);
}

bool CFrpsForSubDlg::ValidateInput()
{
    if (m_checkEnabled.GetCheck() != BST_CHECKED) {
        return true;  // 未启用，无需验证
    }

    CString str;

    // 验证服务器地址
    m_editServer.GetWindowText(str);
    if (str.IsEmpty()) {
        MessageBox(_TR("请输入服务器地址"), _TR("验证失败"), MB_OK | MB_ICONWARNING);
        m_editServer.SetFocus();
        return false;
    }

    // 验证端口
    m_editPort.GetWindowText(str);
    int port = _ttoi(str);
    if (port <= 0 || port > 65535) {
        MessageBox(_TR("服务器端口无效（1-65535）"), _TR("验证失败"), MB_OK | MB_ICONWARNING);
        m_editPort.SetFocus();
        return false;
    }

    // 验证 Token
    m_editToken.GetWindowText(str);
    if (str.IsEmpty()) {
        MessageBox(_TR("请输入认证 Token"), _TR("验证失败"), MB_OK | MB_ICONWARNING);
        m_editToken.SetFocus();
        return false;
    }

    // 验证端口范围（可选，但如果填了就需要验证）
    m_editPortStart.GetWindowText(str);
    int portStart = _ttoi(str);
    m_editPortEnd.GetWindowText(str);
    int portEnd = _ttoi(str);

    if (portStart > 0 || portEnd > 0) {
        if (portStart <= 0 || portStart > 65535) {
            MessageBox(_TR("起始端口无效（1-65535）"), _TR("验证失败"), MB_OK | MB_ICONWARNING);
            m_editPortStart.SetFocus();
            return false;
        }
        if (portEnd <= 0 || portEnd > 65535) {
            MessageBox(_TR("结束端口无效（1-65535）"), _TR("验证失败"), MB_OK | MB_ICONWARNING);
            m_editPortEnd.SetFocus();
            return false;
        }
        if (portStart >= portEnd) {
            MessageBox(_TR("起始端口必须小于结束端口"), _TR("验证失败"), MB_OK | MB_ICONWARNING);
            m_editPortStart.SetFocus();
            return false;
        }
    }

    return true;
}

void CFrpsForSubDlg::OnOK()
{
    if (!ValidateInput()) {
        return;
    }

    SaveSettings();
    CDialogLangEx::OnOK();
}

void CFrpsForSubDlg::OnBnClickedCheckFrpsEnabled()
{
    UpdateControlStates();
}

void CFrpsForSubDlg::OnBnClickedCheckFrpsLocal()
{
    UpdateControlStates();
}

void CFrpsForSubDlg::OnBnClickedBtnShowToken()
{
    m_bShowToken = !m_bShowToken;

    // 切换密码显示模式
    CString currentText;
    m_editToken.GetWindowText(currentText);

    // 获取控件位置
    CRect rect;
    m_editToken.GetWindowRect(&rect);
    ScreenToClient(&rect);

    // 销毁旧控件，创建新控件
    m_editToken.DestroyWindow();

    DWORD style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL;
    if (!m_bShowToken) {
        style |= ES_PASSWORD;
    }

    m_editToken.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), currentText, style, rect, this, IDC_EDIT_FRPS_TOKEN);
    m_editToken.SetFont(GetFont());

    // 更新按钮文字（显示时用眼睛图标，隐藏时用星号）
    m_btnShowToken.SetWindowText(m_bShowToken ? _T("○") : _T("*"));
}

// 静态方法：获取 FRPS 配置
FrpsConfig CFrpsForSubDlg::GetFrpsConfig()
{
    FrpsConfig config;
    config.enabled = THIS_CFG.GetInt("frps_for_sub", "enabled", 0) != 0;
    config.localFrps = THIS_CFG.GetInt("frps_for_sub", "local_frps", 0) != 0;
    config.server = THIS_CFG.GetStr("frps_for_sub", "server", "");
    config.port = THIS_CFG.GetInt("frps_for_sub", "port", 7000);
    config.token = THIS_CFG.GetStr("frps_for_sub", "token", "");
    config.portStart = THIS_CFG.GetInt("frps_for_sub", "port_start", 20000);
    config.portEnd = THIS_CFG.GetInt("frps_for_sub", "port_end", 29999);
    config.authMode = (FrpAuthMode)THIS_CFG.GetInt("frps_for_sub", "auth_mode", FRP_AUTH_PRIVILEGE_KEY);
    return config;
}

// 静态方法：检查 FRPS 是否已配置且启用
bool CFrpsForSubDlg::IsFrpsConfigured()
{
    FrpsConfig config = GetFrpsConfig();
    return config.enabled && !config.server.empty() && !config.token.empty() && config.port > 0;
}

// 静态方法：查找下一个可用端口（不保存）
int CFrpsForSubDlg::FindNextAvailablePort()
{
    FrpsConfig frpsCfg = GetFrpsConfig();
    int portStart = frpsCfg.portStart > 0 ? frpsCfg.portStart : 20000;
    int portEnd = frpsCfg.portEnd > 0 ? frpsCfg.portEnd : 29999;

    // 使用 config 类访问 frp_ports.ini
    ::config portsCfg(GetFrpPortsPath());

    // 查找下一个可用端口
    for (int port = portStart; port <= portEnd; port++) {
        char portStr[16];
        sprintf_s(portStr, "%d", port);
        std::string owner = portsCfg.GetStr("ports", portStr, "");
        if (owner.empty()) {
            return port;  // 找到可用端口，不保存
        }
    }

    return -1;  // 没有可用端口
}

// 静态方法：记录端口分配（保存到配置文件）
void CFrpsForSubDlg::RecordPortAllocation(int port, const std::string& serialNumber)
{
    if (port <= 0 || serialNumber.empty()) {
        return;
    }

    ::config portsCfg(GetFrpPortsPath());
    char portStr[16];
    sprintf_s(portStr, "%d", port);
    portsCfg.SetStr("ports", portStr, serialNumber);
}

// 静态方法：检查端口是否已分配
bool CFrpsForSubDlg::IsPortAllocated(int port)
{
    ::config portsCfg(GetFrpPortsPath());
    char portStr[16];
    sprintf_s(portStr, "%d", port);
    return !portsCfg.GetStr("ports", portStr, "").empty();
}

// 静态方法：获取端口对应的序列号
std::string CFrpsForSubDlg::GetPortOwner(int port)
{
    ::config portsCfg(GetFrpPortsPath());
    char portStr[16];
    sprintf_s(portStr, "%d", port);
    return portsCfg.GetStr("ports", portStr, "");
}
