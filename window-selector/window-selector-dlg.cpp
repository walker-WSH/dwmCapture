
// window-selectorDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "window-selector.h"
#include "window-selector-dlg.h"
#include "afxdialogex.h"
#include <assert.h>

#pragma comment(lib, "Dwmapi.lib")

CWindowSelectorDlg *g_pMainDlg = nullptr;
CFont g_textFont;

WindowItemInfo::WindowItemInfo(HWND wnd, const std::wstring &text, const std::wstring &path)
	: hWnd(wnd), title(text)
{
	SHFILEINFO fiLarge = {};
	SHGetFileInfo(path.c_str(), 0, &fiLarge, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON);
	hIcon = fiLarge.hIcon;

	hDwmReg = RegisterThumbWindow(wnd, g_pMainDlg->GetSafeHwnd());

	//------------------------------------------------------------------------------------
	pStaticLabel = new CStatic();
	pStaticLabel->Create(text.c_str(),
			     WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE | SS_ELLIPSISMASK,
			     drawRegion, g_pMainDlg);
	pStaticLabel->SetFont(&g_textFont, true);

	//------------------------------------------------------------------------------------
	pStaticIcon = new CStatic();
	pStaticIcon->Create(L"", WS_CHILD | WS_VISIBLE | SS_ICON | SS_CENTERIMAGE, drawRegion,
			    g_pMainDlg);
	pStaticIcon->SetIcon(hIcon);
}

WindowItemInfo::~WindowItemInfo()
{
	if (hDwmReg) {
		DwmUnregisterThumbnail(hDwmReg);
		hDwmReg = nullptr;
	}

	if (pStaticLabel) {
		pStaticLabel->DestroyWindow();
		delete pStaticLabel;
		pStaticLabel = nullptr;
	}

	if (pStaticIcon) {
		pStaticIcon->DestroyWindow();
		delete pStaticIcon;
		pStaticIcon = nullptr;
	}

	if (hIcon) {
		DestroyIcon(hIcon);
		hIcon = nullptr;
	}
}

CWindowSelectorDlg::CWindowSelectorDlg(CWnd *pParent /*=nullptr*/)
	: CDialogEx(IDD_WINDOWSELECTOR_DIALOG, pParent)
{
	auto hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));

	g_textFont.CreatePointFont(80, L"Arial", NULL);
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CWindowSelectorDlg::~CWindowSelectorDlg()
{
	g_textFont.DeleteObject();
	CoUninitialize();
}

void CWindowSelectorDlg::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWindowSelectorDlg, CDialogEx)
ON_WM_PAINT()
ON_WM_ERASEBKGND()
ON_WM_DESTROY()
ON_WM_TIMER()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

BOOL CWindowSelectorDlg::OnInitDialog()
{
	g_pMainDlg = this;

	CDialogEx::OnInitDialog();

	SetWindowText(L"Window Selector");
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	MoveWindow(0, 0, MAIN_DLG_WIDTH, MAIN_DLG_HEIGHT);
	CenterWindow();

	refreshWindows();

	m_hHook = SetWinEventHook(
		EVENT_OBJECT_DESTROY, EVENT_OBJECT_UNCLOAKED, nullptr,
		[](HWINEVENTHOOK eventHook, DWORD event, HWND hwnd, LONG objectId, LONG childId,
		   DWORD eventThreadId, DWORD eventTimeInMilliseconds) {
			if (objectId == OBJID_WINDOW && event == EVENT_OBJECT_DESTROY &&
			    childId == CHILDID_SELF) {
				g_pMainDlg->removeWindow(hwnd);
			}

			if (objectId == OBJID_WINDOW && childId == CHILDID_SELF &&
			    hwnd != nullptr && ::GetAncestor(hwnd, GA_ROOT) == hwnd &&
			    (event == EVENT_OBJECT_SHOW || event == EVENT_OBJECT_UNCLOAKED)) {
				if (IsWindowAvailable(hwnd)) {
					g_pMainDlg->refreshWindows();
				}
			}
		},
		0, 0, WINEVENT_OUTOFCONTEXT);

	return TRUE;
}

void CWindowSelectorDlg::OnDestroy()
{
	if (m_hHook) {
		UnhookWinEvent(m_hHook);
		m_hHook = nullptr;
	}

	m_vecWindows.clear();
	g_pMainDlg = nullptr;

	CDialogEx::OnDestroy();
}

void drawBorder(CPaintDC &dc, RECT &rc, DWORD color)
{
	CBrush clrBrush(color);

	{
		auto temp = rc;
		temp.right = rc.left + ITEM_BORDER_SIZE;
		dc.FillRect(&temp, &clrBrush);
	}

	{
		auto temp = rc;
		temp.bottom = rc.top + ITEM_BORDER_SIZE;
		dc.FillRect(&temp, &clrBrush);
	}

	{
		auto temp = rc;
		temp.left = rc.right - ITEM_BORDER_SIZE;
		dc.FillRect(&temp, &clrBrush);
	}

	{
		auto temp = rc;
		temp.top = rc.bottom - ITEM_BORDER_SIZE;
		dc.FillRect(&temp, &clrBrush);
	}
}

void CWindowSelectorDlg::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(&rect);

	CBrush clrBrush(0xF0F0F0);
	dc.FillRect(&rect, &clrBrush);

	for (auto &item : m_vecWindows) {
		RECT rc = item->drawRegion;
		rc.top -= EXE_ICON_MARGIN;
		rc.left -= EXE_ICON_MARGIN;
		rc.right += EXE_ICON_MARGIN;
		rc.bottom += (EXE_ICON_MARGIN + EXE_ICON_SIZE + EXE_ICON_MARGIN);

		drawBorder(dc, rc, item->selected ? 0x00FF00 : 0xC0C0C0);
	}
}

BOOL CWindowSelectorDlg::OnEraseBkgnd(CDC *pDC)
{
	return TRUE;
}

BOOL CALLBACK enumWindowList(HWND window, LPARAM lParam)
{
	auto output = (std::vector<std::shared_ptr<WindowItemInfo>> *)lParam;

	if (!IsWindowAvailable(window))
		return TRUE;

	auto title = GetWindowTitle(window);
	if (title.empty())
		return TRUE;

	auto path = GetExePath(window);
	if (path.empty())
		return TRUE;

	wchar_t temp[MAX_PATH] = {0};
	GetClassNameW(window, temp, MAX_PATH);

	if (path.find(L"explorer.exe") != std::wstring::npos &&
	    std::wstring(temp) == L"Xaml_WindowedPopupClass") {
		return TRUE;
	}

	if (!g_pMainDlg->isWndExist(window)) {
		auto item = std::make_shared<WindowItemInfo>(window, title, path);
		output->push_back(item);
	}

	return TRUE;
}

bool CWindowSelectorDlg::isWndExist(HWND window)
{
	for (auto &item : m_vecWindows) {
		if (item->hWnd == window)
			return true;
	}

	return false;
}

void CWindowSelectorDlg::refreshWindows()
{
	auto itr = m_vecWindows.begin();
	while (itr != m_vecWindows.end()) {
		auto item = *itr;
		if (!IsWindow(item->hWnd)) {
			itr = m_vecWindows.erase(itr);
			continue;
		}

		++itr;
	}

	EnumWindows(enumWindowList, (LPARAM)&m_vecWindows);

	if (!m_vecWindows.empty())
		initLayout();

	SetTimer(TIMER_EVENT, 20, 0);
	Invalidate(FALSE);
}

void CWindowSelectorDlg::removeWindow(HWND wnd)
{
	bool found = false;

	{
		auto itr = m_vecWindows.begin();
		while (itr != m_vecWindows.end()) {
			if ((*itr)->hWnd == wnd) {
				found = true;
				m_vecWindows.erase(itr);
				break;
			}

			++itr;
		}
	}

	if (found) {
		initLayout();
		Invalidate(FALSE);
	}
}

void CWindowSelectorDlg::initWindows()
{
	if (m_vecWindows.empty())
		return;

	initLayout();
	Invalidate(FALSE);
}

RECT getBestRegion(const RECT &drawDest)
{
	RECT realDrawDest = drawDest;

	float destWidth = 16.f;
	float destHeight = 9.f;
	auto cx = drawDest.right - drawDest.left;
	auto cy = drawDest.bottom - drawDest.top;
	float wndRadio = float(cx) / float(cy);
	float frameRadio = destWidth / destHeight;
	if (wndRadio > frameRadio) {
		float radio = float(cy) / destHeight;
		auto destCx = radio * destWidth;

		realDrawDest.left += ((drawDest.right - drawDest.left) - (LONG)destCx) / 2;
		realDrawDest.right = realDrawDest.left + (LONG)destCx;
	} else {
		float radio = float(cx) / destWidth;
		auto destCy = radio * destHeight;

		realDrawDest.top += ((drawDest.bottom - drawDest.top) - (LONG)destCy) / 2;
		realDrawDest.bottom = realDrawDest.top + (LONG)destCy;
	}

	return realDrawDest;
}

void CWindowSelectorDlg::initLayout()
{
	m_xVideoCount = m_yVideoCount = 1;

	DWORD64 count = m_vecWindows.size();
	while (1) {
		if (m_xVideoCount * m_yVideoCount >= count) {
			break;
		}

		++m_xVideoCount;
		if (m_xVideoCount * m_yVideoCount >= count) {
			break;
		}

		++m_xVideoCount;
		if (m_xVideoCount * m_yVideoCount >= count) {
			break;
		}

		++m_yVideoCount;
		if (m_xVideoCount * m_yVideoCount >= count) {
			break;
		}
	}

	RECT rc;
	GetClientRect(&rc);
	rc.left += EXE_ICON_MARGIN;
	rc.top += EXE_ICON_MARGIN;
	rc.right -= EXE_ICON_MARGIN;
	rc.bottom -= EXE_ICON_SIZE;

	DWORD64 width = (DWORD64)rc.right - (DWORD64)rc.left;
	DWORD64 height = (DWORD64)rc.bottom - (DWORD64)rc.top;

	DWORD64 xStep = width / m_xVideoCount;
	DWORD64 yStep = height / m_yVideoCount;

	static const auto nXGridMargin = 10;
	static const auto nYGridMargin = 40;

	DWORD64 index = 0;
	RECT rcItem;
	for (DWORD64 y = 0; y < m_yVideoCount && index < count; y++) {
		for (DWORD64 x = 0; x < m_xVideoCount && index < count; x++) {
			rcItem.left = rc.left + LONG(x * xStep + nXGridMargin);
			rcItem.right = rc.left + LONG(x * xStep + (xStep - nXGridMargin));

			rcItem.top = rc.top + LONG(y * yStep + nYGridMargin);
			rcItem.bottom = rc.top + LONG(y * yStep + (yStep - nYGridMargin));

			auto &item = m_vecWindows[(unsigned)index];
			item->drawRegion = getBestRegion(rcItem);

			++index;
		}
	}

	bool itemRemoved;
	updateLayout(true, itemRemoved);
}

void CWindowSelectorDlg::updateLayout(bool moveUI, bool &itemRemoved)
{
	itemRemoved = false;

	for (auto &item : m_vecWindows) {
		if (!item->hDwmReg)
			continue;

		if (!::IsWindow(item->hWnd)) {
			if (item->hDwmReg) {
				itemRemoved = true;

				DwmUnregisterThumbnail(item->hDwmReg);
				item->hDwmReg = nullptr;
			}

			continue;
		}

		DWM_THUMBNAIL_PROPERTIES dskThumbProps;
		dskThumbProps.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_VISIBLE |
					DWM_TNP_SOURCECLIENTAREAONLY | DWM_TNP_OPACITY;
		dskThumbProps.fSourceClientAreaOnly = FALSE;
		dskThumbProps.fVisible = TRUE;
		dskThumbProps.rcDestination = item->drawRegion;
		dskThumbProps.opacity = 255;

		DwmUpdateThumbnailProperties(item->hDwmReg, &dskThumbProps);

		if (moveUI) {
			RECT rc = item->drawRegion;
			rc.top = item->drawRegion.bottom + EXE_ICON_MARGIN;
			rc.bottom = rc.top + EXE_ICON_SIZE;

			RECT rcIcon = rc;
			rcIcon.right = rcIcon.left + EXE_ICON_SIZE;

			RECT rcTitle = rc;
			rcTitle.left = rcIcon.right + EXE_ICON_MARGIN;

			item->pStaticIcon->MoveWindow(&rcIcon);
			item->pStaticLabel->MoveWindow(&rcTitle);
		}
	}

	if (moveUI) {
		Invalidate(FALSE);
	}
}

void CWindowSelectorDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_EVENT) {
		bool itemRemoved;
		updateLayout(false, itemRemoved);
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CWindowSelectorDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	HWND selectedWnd = 0;

	for (auto &item : m_vecWindows) {
		RECT rc = item->drawRegion;
		if (point.x > rc.left && point.x < rc.right && point.y > rc.top &&
		    point.y < rc.bottom) {
			item->selected = true;
			selectedWnd = item->hWnd;
			break;
		}
	}

	if (selectedWnd) {
		for (auto &item : m_vecWindows) {
			if (item->selected && item->hWnd != selectedWnd) {
				item->selected = false;
			}
		}

		Invalidate(FALSE);
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CWindowSelectorDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	for (auto &item : m_vecWindows) {
		if (item->selected && IsWindow(item->hWnd)) {
			::MessageBoxW(0, item->title.c_str(), 0, 0);
			break;
		}
	}

	CDialogEx::OnLButtonDblClk(nFlags, point);
}
