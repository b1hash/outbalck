// IPHistoryDlg.h - IP 历史记录对话框
#pragma once

#include "resource.h"
#include "LangManager.h"
#include <vector>
#include <string>

struct IPRecord {
    std::string ip;
    std::string machineName;
    std::string timestamp;
    std::string formattedDate;
    int daysAgo;
};

class CIPHistoryDlg : public CDialogLangEx
{
public:
    CIPHistoryDlg(CWnd* pParent = nullptr);
    virtual ~CIPHistoryDlg();

    enum { IDD = IDD_DIALOG_IP_HISTORY };

    // 设置数据
    void SetTitle(const CString& title) { m_strTitle = title; }
    void SetRecords(const std::vector<IPRecord>& records) { m_Records = records; }
    void SetRemovedCount(int count) { m_nRemovedCount = count; }

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

private:
    CListBox m_ListBox;
    CString m_strTitle;
    std::vector<IPRecord> m_Records;
    int m_nRemovedCount;

    void PopulateList();
};
