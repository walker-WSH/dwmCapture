
// MFCApplication3Dlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication3.h"
#include "MFCApplication3Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCApplication3Dlg 对话框



CMFCApplication3Dlg::CMFCApplication3Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION3_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_cbxWndList);
	DDX_Control(pDX, IDOK, m_btnStart);
}

BEGIN_MESSAGE_MAP(CMFCApplication3Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CMFCApplication3Dlg::OnBnClickedOk)
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication3Dlg::OnBnClickedButton1)
	ON_NOTIFY(NM_THEMECHANGED, IDC_COMBO1, &CMFCApplication3Dlg::OnNMThemeChangedCombo1)
END_MESSAGE_MAP()


// CMFCApplication3Dlg 消息处理程序

BOOL CMFCApplication3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	OnBnClickedButton1();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCApplication3Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCApplication3Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCApplication3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


#include <dwmapi.h>
#pragma comment(lib,"Dwmapi.lib")

HRESULT RegisterThumbWindow(HWND hWndSrc, HWND hWndDst)
{
	HTHUMBNAIL thumbnail = NULL;
	HRESULT hr = DwmRegisterThumbnail(hWndDst, hWndSrc, &thumbnail);
	if (FAILED(hr))
		return hr; // 如果窗口句柄不存在 或传入了非顶层窗口的句柄 此处会出错

	RECT dest;
	GetClientRect(hWndDst, &dest);

	DWM_THUMBNAIL_PROPERTIES dskThumbProps;
	dskThumbProps.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_VISIBLE | DWM_TNP_SOURCECLIENTAREAONLY | DWM_TNP_OPACITY; // 标识哪些字段已经设置了有效值
	dskThumbProps.fSourceClientAreaOnly = FALSE;
	dskThumbProps.fVisible = TRUE;
	dskThumbProps.rcDestination = dest;
	dskThumbProps.opacity = 255;

	hr = DwmUpdateThumbnailProperties(thumbnail, &dskThumbProps);
	if (FAILED(hr))
		OutputDebugStringA("error");

	// DwmUnregisterThumbnail(thumbnail);
	return hr;
}


void CMFCApplication3Dlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialogEx::OnLButtonDown(nFlags, point);
}


void CMFCApplication3Dlg::OnTimer(UINT_PTR nIDEvent)
{

	CDialogEx::OnTimer(nIDEvent);
}

#include <string>
std::wstring GetWindowTitle(HWND hwnd)
{
	wchar_t title[MAX_PATH] = { 0 };
	ULONG_PTR copyCount = 0;
	LRESULT res = SendMessageTimeoutW(hwnd, WM_GETTEXT, MAX_PATH, (LPARAM)title,
		SMTO_ABORTIFHUNG, 2000, &copyCount);
	if (0 != res && copyCount > 0)
		return std::wstring(title);
	else
		return std::wstring();
}

bool IsWindowAvailable(HWND window)
{
	if (!IsWindowVisible(window))
		return false;

	if (IsIconic(window))
		return false;

	DWORD dwDestProcessID = 0;
	::GetWindowThreadProcessId(window, &dwDestProcessID);
	if (dwDestProcessID == GetCurrentProcessId())
		return false;

	int isCloaked = 0;
	HRESULT hr = DwmGetWindowAttribute(window, DWMWA_CLOAKED, &isCloaked, sizeof(isCloaked));
	if (SUCCEEDED(hr) && isCloaked)
		return false;

	RECT rect;
	GetClientRect(window, &rect);
	if (rect.bottom == 0 || rect.right == 0)
		return false;

	DWORD exStyle = (DWORD)GetWindowLongPtr(window, GWL_EXSTYLE);
	if (exStyle & WS_EX_TOOLWINDOW)
		return false;

	DWORD style = (DWORD)GetWindowLongPtr(window, GWL_STYLE);
	if (style & WS_CHILD)
		return false;

	return true;
}

BOOL CALLBACK EnumWindowCallback(HWND window, LPARAM lParam)
{
	auto cbx = (CComboBox*)lParam;

	if (!IsWindowAvailable(window))
		return TRUE;

	auto title = GetWindowTitle(window);
	if (title.empty())
		return TRUE;

	auto index = cbx->AddString(title.c_str());
	cbx->SetItemData(index, (DWORD_PTR)window);
	return TRUE;
}

void CMFCApplication3Dlg::OnBnClickedButton1()
{
	m_cbxWndList.ResetContent();
	EnumWindows(EnumWindowCallback, (LPARAM)&m_cbxWndList);
	m_cbxWndList.SetCurSel(0);
}

void CMFCApplication3Dlg::OnBnClickedOk()
{
	int sel = m_cbxWndList.GetCurSel();
	if (sel < 0)
		return;

	HWND src = (HWND)m_cbxWndList.GetItemData(sel);

	RegisterThumbWindow(src, m_hWnd);
	SetTimer(2000, 20, 0);
}


void CMFCApplication3Dlg::OnNMThemeChangedCombo1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// 该功能要求使用 Windows XP 或更高版本。
	// 符号 _WIN32_WINNT 必须 >= 0x0501。
	// TODO: 在此添加控件通知处理程序代码
	if (m_cbxWndList.GetCurSel() >= 0) {
		m_btnStart.EnableWindow(TRUE);
	}
	else {
		m_btnStart.EnableWindow(FALSE);
	}


	*pResult = 0;
}
