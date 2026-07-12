// NetworkDlg.h - 网络配置对话框
// 用于配置 IP 白名单和黑名单

#pragma once

#include "resource.h"
#include "LangManager.h"
#include "../../common/IPWhitelist.h"
#include "../../common/IPBlacklist.h"

class CNetworkDlg : public CDialogLangEx
{
public:
    CNetworkDlg(CWnd* pParent = nullptr);
    virtual ~CNetworkDlg();

    enum { IDD = IDD_DIALOG_NETWORK };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

    // 控件
    CListBox m_ListWhitelist;
    CListBox m_ListBlacklist;
    CEdit m_EditIP;

    // 阈值设置
    UINT m_nDllLimitSeconds;
    UINT m_nDllLimitCount;
    UINT m_nBanWindow;
    UINT m_nBanMaxConn;
    UINT m_nBanDuration;

    // 消息处理
    afx_msg void OnBnClickedAddWhitelist();
    afx_msg void OnBnClickedDelWhitelist();
    afx_msg void OnBnClickedAddBlacklist();
    afx_msg void OnBnClickedDelBlacklist();
    afx_msg void OnBnClickedOk();

private:
    void LoadLists();
    void SaveLists();
    void LoadThresholds();
    void SaveThresholds();
    bool IsValidIP(const CString& ip);
    CString GetInputIP();
};
