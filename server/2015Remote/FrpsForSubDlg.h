#pragma once

#include "resource.h"
#include "LangManager.h"

// FRP 认证模式
enum FrpAuthMode {
    FRP_AUTH_TOKEN = 0,      // 官方 FRP: 直接使用 token
    FRP_AUTH_PRIVILEGE_KEY = 1  // 自定义 FRP: 使用 privilegeKey = MD5(token + timestamp)
};

// FRPS 配置结构体
struct FrpsConfig {
    bool enabled;
    bool localFrps;          // FRPS 运行在本机
    std::string server;
    int port;
    std::string token;
    int portStart;
    int portEnd;
    FrpAuthMode authMode;    // 认证模式

    FrpsConfig() : enabled(false), localFrps(false), port(7000), portStart(20000), portEnd(29999), authMode(FRP_AUTH_PRIVILEGE_KEY) {}
};

// 下级 FRP 代理设置对话框
class CFrpsForSubDlg : public CDialogLangEx
{
public:
    CFrpsForSubDlg(CWnd* pParent = nullptr);
    virtual ~CFrpsForSubDlg();

    enum { IDD = IDD_DIALOG_FRPS_FOR_SUB };

    // 获取当前配置
    static FrpsConfig GetFrpsConfig();
    // 检查 FRPS 是否已配置且启用
    static bool IsFrpsConfigured();
    // 查找下一个可用端口（不保存）
    static int FindNextAvailablePort();
    // 记录端口分配（保存到配置文件）
    static void RecordPortAllocation(int port, const std::string& serialNumber);
    // 检查端口是否已分配
    static bool IsPortAllocated(int port);
    // 获取端口对应的序列号
    static std::string GetPortOwner(int port);
    // 获取 FRP 端口分配文件路径
    static std::string GetFrpPortsPath();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK();

    DECLARE_MESSAGE_MAP()

    afx_msg void OnBnClickedCheckFrpsEnabled();
    afx_msg void OnBnClickedCheckFrpsLocal();
    afx_msg void OnBnClickedBtnShowToken();

private:
    void LoadSettings();
    void SaveSettings();
    void UpdateControlStates();
    bool ValidateInput();

private:
    // 控件变量
    CButton m_checkEnabled;
    CButton m_checkLocalFrps;
    CEdit m_editServer;
    CEdit m_editPort;
    CEdit m_editToken;
    CButton m_btnShowToken;
    CEdit m_editPortStart;
    CEdit m_editPortEnd;
    CButton m_radioOfficial;
    CButton m_radioCustom;

    // 数据变量
    FrpsConfig m_config;
    bool m_bShowToken;
};
