#pragma once

#include "resource.h"
#include "LangManager.h"

// CInputDialog 对话框

class CInputDialog : public CDialogLangEx
{
    DECLARE_DYNAMIC(CInputDialog)

public:
    CInputDialog(CWnd* pParent = nullptr);   // 标准构造函数
    virtual ~CInputDialog();

    BOOL Init(LPCTSTR caption, LPCTSTR prompt);

    void Init2(LPCTSTR name, LPCTSTR defaultValue);
    void Init3(LPCTSTR name, LPCTSTR defaultValue);  // 第三个输入框
    void SetHistoryKey(LPCTSTR historyKey);          // 设置历史记录的配置键名

// 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_DIALOG_INPUT };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

    DECLARE_MESSAGE_MAP()

    HICON m_hIcon;
    CString m_sCaption;
    CString m_sPrompt;
    CString m_sHistoryKey;  // 历史记录键名
    CComboBox m_ComboInput; // 主输入框 (ComboBox)

    void LoadHistory();     // 从配置加载历史到下拉列表
    void SaveHistory();     // 保存当前输入到历史

public:
    CString m_str;
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    CStatic m_Static2thInput;
    CEdit m_Edit2thInput;
    CString m_sItemName;
    CString m_sSecondInput;
    CStatic m_Static3rdInput;
    CEdit m_Edit3rdInput;
    CString m_sItemName3;
    CString m_sThirdInput;
    CStatic m_StaticTipInfo;
    CString m_sTipInfo;
};

typedef CInputDialog CInputDlg;
