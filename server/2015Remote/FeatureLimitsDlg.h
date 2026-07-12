#pragma once

#include "resource.h"
#include "LangManager.h"
#include "FeatureFlags.h"
#include <afxcmn.h>
#include <vector>

// 功能项定义
struct FeatureItem {
    const char* name;           // 显示名称
    uint64_t flag;              // 运行时标志位
    int compileMacro;           // 编译时宏值 (0或1)
};

// 下级功能限制对话框
class CFeatureLimitsDlg : public CDialogLangEx
{
public:
    CFeatureLimitsDlg(CWnd* pParent = nullptr);
    virtual ~CFeatureLimitsDlg();

    enum { IDD = IDD_FEATURE_LIMITS };

    // 设置继承的限制（上级已禁用的功能）
    void SetInheritedFlags(const FeatureFlags* inherited);

    // 获取用户选择的限制
    uint64_t GetMenuFlags() const { return m_MenuFlags; }
    uint64_t GetToolbarFlags() const { return m_ToolbarFlags; }
    uint64_t GetContextFlags() const { return m_ContextFlags; }

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK();

    DECLARE_MESSAGE_MAP()

    afx_msg void OnTcnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);

private:
    void InitMenuItems();
    void InitToolbarItems();
    void InitContextItems();
    void PopulateList(int tabIndex);
    void CollectFlags();

private:
    CTabCtrl m_TabCtrl;
    CCheckListBox m_CheckList;

    // 继承的限制
    uint64_t m_InheritedMenuFlags;
    uint64_t m_InheritedToolbarFlags;
    uint64_t m_InheritedContextFlags;

    // 用户选择的限制
    uint64_t m_MenuFlags;
    uint64_t m_ToolbarFlags;
    uint64_t m_ContextFlags;

    // 功能项列表
    std::vector<FeatureItem> m_MenuItems;
    std::vector<FeatureItem> m_ToolbarItems;
    std::vector<FeatureItem> m_ContextItems;

    // 当前Tab索引
    int m_CurrentTab;
};
