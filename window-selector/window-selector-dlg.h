
// window-selectorDlg.h: 头文件
//

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <afxwin.h>
#include "tool.hpp"

static const auto MAIN_DLG_WIDTH = 1080;
static const auto MAIN_DLG_HEIGHT = 500;

static const auto ITEM_BORDER_SIZE = 2;

static const auto EXE_ICON_SIZE = 32;
static const auto EXE_ICON_MARGIN = 5;

class CWindowSelectorDlg;
struct WindowItemInfo {
	HWND hWnd = 0;
	std::wstring title = L"";

	RECT drawRegion = {0, 0, 0, 0};
	bool selected = false;

	HTHUMBNAIL hDwmReg = nullptr;
	HICON hIcon = nullptr;
	CStatic *pStaticIcon = nullptr;
	CButton *pStaticLabel = nullptr;

	WindowItemInfo(HWND wnd, const std::wstring &text, const std::wstring &exe);
	virtual ~WindowItemInfo();
};

class CWindowSelectorDlg : public CDialogEx {
public:
	CWindowSelectorDlg(CWnd *pParent = nullptr);
	~CWindowSelectorDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WINDOWSELECTOR_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange *pDX);

protected:
	HICON m_hIcon;

	bool m_bActived = false;

	HWINEVENTHOOK m_hHook = nullptr;

	std::vector<std::shared_ptr<WindowItemInfo>> m_vecWindows;
	DWORD64 m_xVideoCount = 1;
	DWORD64 m_yVideoCount = 1;

public:
	bool isWndExist(HWND window);
	void refreshWindows();
	void removeWindow(HWND wnd);
	void initWindows();
	void initLayout();
	void updateLayout(bool moveUI, bool &itemRemoved);

public:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	virtual BOOL PreTranslateMessage(MSG *pMsg);
};
