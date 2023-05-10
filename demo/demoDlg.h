
// demoDlg.h: 头文件
//

#pragma once

#include <vector>
#include "tool.hpp"
#include "lib-window-selector.h"

using namespace selector;

static const auto MSG_ON_RESULT = WM_USER + 100;

class SelectEvent : public IWindowSelectorEvent {
	HWND m_hMainWnd = 0;

public:
	SelectEvent(HWND mainWnd) : m_hMainWnd(mainWnd) {}

	virtual void onSelectResult(HWND hWnd)
	{
		if (::IsWindow(hWnd)) {
			PostMessage(m_hMainWnd, MSG_ON_RESULT, (WPARAM)hWnd, 0);
		}
	}
};

// CdemoDlg 对话框
class CdemoDlg : public CDialogEx {
	// 构造
public:
	CdemoDlg(CWnd *pParent = nullptr); // 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DEMO_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange *pDX); // DDX/DDV 支持

	// 实现
protected:
	HICON m_hIcon;

	std::shared_ptr<IWindowSelector> m_pSelector = nullptr;
	std::vector<std::shared_ptr<SelectEvent>> m_vecEvents;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonSelect();
	afx_msg void OnDestroy();
	LRESULT OnSelectResult(WPARAM, LPARAM);
};
