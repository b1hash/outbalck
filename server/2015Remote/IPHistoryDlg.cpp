// IPHistoryDlg.cpp - IP 历史记录对话框实现

#include "stdafx.h"
#include "IPHistoryDlg.h"
#include "afxdialogex.h"

CIPHistoryDlg::CIPHistoryDlg(CWnd* pParent /*=nullptr*/)
    : CDialogLangEx(IDD_DIALOG_IP_HISTORY, pParent)
    , m_nRemovedCount(0)
{
}

CIPHistoryDlg::~CIPHistoryDlg()
{
}

void CIPHistoryDlg::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_IP_HISTORY, m_ListBox);
}

BEGIN_MESSAGE_MAP(CIPHistoryDlg, CDialogLangEx)
END_MESSAGE_MAP()

BOOL CIPHistoryDlg::OnInitDialog()
{
    __super::OnInitDialog();

    // 设置标题
    if (!m_strTitle.IsEmpty()) {
        SetWindowText(m_strTitle);
    }

    // 多语言
    SetDlgItemText(IDOK, _TR("确定"));

    // 填充列表
    PopulateList();

    // 设置摘要
    CString summary;
    if (m_nRemovedCount > 0) {
        summary.Format(_TR("共 %d 条记录（已清理 %d 条过期记录）"),
            (int)m_Records.size(), m_nRemovedCount);
    } else {
        summary.Format(_TR("共 %d 条记录"), (int)m_Records.size());
    }
    SetDlgItemText(IDC_STATIC_IP_SUMMARY, summary);

    return TRUE;
}

void CIPHistoryDlg::PopulateList()
{
    m_ListBox.ResetContent();

    // 设置水平滚动范围
    CDC* pDC = m_ListBox.GetDC();
    int maxWidth = 0;

    int index = 1;
    for (const auto& record : m_Records) {
        CString line;

        if (!record.machineName.empty()) {
            if (!record.formattedDate.empty()) {
                line.Format(_TR("%d. %s  [%s]  (最后活跃: %s)"),
                    index,
                    CString(record.ip.c_str()).GetString(),
                    CString(record.machineName.c_str()).GetString(),
                    CString(record.formattedDate.c_str()).GetString());
            } else {
                line.Format(_T("%d. %s  [%s]"),
                    index,
                    CString(record.ip.c_str()).GetString(),
                    CString(record.machineName.c_str()).GetString());
            }
        } else {
            if (!record.formattedDate.empty()) {
                line.Format(_TR("%d. %s  (最后活跃: %s)"),
                    index,
                    CString(record.ip.c_str()).GetString(),
                    CString(record.formattedDate.c_str()).GetString());
            } else {
                line.Format(_T("%d. %s"),
                    index,
                    CString(record.ip.c_str()).GetString());
            }
        }

        m_ListBox.AddString(line);

        // 计算文本宽度
        if (pDC) {
            CSize size = pDC->GetTextExtent(line);
            if (size.cx > maxWidth) {
                maxWidth = size.cx;
            }
        }

        index++;
    }

    // 设置水平滚动范围
    if (pDC) {
        m_ListBox.ReleaseDC(pDC);
        m_ListBox.SetHorizontalExtent(maxWidth + 20);
    }
}
