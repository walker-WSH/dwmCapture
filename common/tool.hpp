#pragma once
#include <Windows.h>
#include <string>
#include <psapi.h>
#include <dwmapi.h>

static auto TIMER_EVENT = 2000;

static std::wstring GetExePath(HWND hWnd)
{
	DWORD pid = 0;
	::GetWindowThreadProcessId(hWnd, &pid);

	HANDLE phdl = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if (!phdl)
		return L"";

	wchar_t path[MAX_PATH];
	DWORD len = MAX_PATH;
	BOOL ret = QueryFullProcessImageNameW(phdl, 0, path,
					      &len); // 获取的exe路径 就是C:\test\test.exe

	::CloseHandle(phdl);
	return (ret ? path : L"");
}

static std::wstring GetWindowTitle(HWND hwnd)
{
	wchar_t title[MAX_PATH] = {0};
	ULONG_PTR copyCount = 0;
	LRESULT res = SendMessageTimeoutW(hwnd, WM_GETTEXT, MAX_PATH, (LPARAM)title,
					  SMTO_ABORTIFHUNG, 2000, &copyCount);
	if (0 != res && copyCount > 0)
		return std::wstring(title);
	else
		return std::wstring();
}

static bool IsWindowAvailable(HWND window)
{
	if (!IsWindow(window) || !IsWindowVisible(window))
		return false;

	auto title = GetWindowTitle(window);
	if (title.empty())
		return false;

	DWORD dwDestProcessID = 0;
	::GetWindowThreadProcessId(window, &dwDestProcessID);
	if (dwDestProcessID == GetCurrentProcessId())
		return false;

	int isCloaked = 0;
	HRESULT hr = DwmGetWindowAttribute(window, DWMWA_CLOAKED, &isCloaked, sizeof(isCloaked));
	if (SUCCEEDED(hr) && isCloaked)
		return false;

	DWORD exStyle = (DWORD)GetWindowLongPtr(window, GWL_EXSTYLE);
	if (exStyle & WS_EX_TOOLWINDOW)
		return false;

	DWORD style = (DWORD)GetWindowLongPtr(window, GWL_STYLE);
	if (style & WS_CHILD)
		return false;

	return true;
}

static void BringWndToTop(HWND hWnd)
{
	if (::IsWindow(hWnd) == FALSE)
		return;

	DWORD dwForeID = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
	DWORD dwCurID = GetCurrentThreadId();

	AttachThreadInput(dwCurID, dwForeID, TRUE);
	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetForegroundWindow(hWnd);
	AttachThreadInput(dwCurID, dwForeID, FALSE);
}

static HTHUMBNAIL RegisterThumbWindow(HWND hWndSrc, HWND hWndDst)
{
	HTHUMBNAIL thumbnail = NULL;
	HRESULT hr = DwmRegisterThumbnail(hWndDst, hWndSrc, &thumbnail);
	if (FAILED(hr))
		return 0; // 如果窗口句柄不存在 或传入了非顶层窗口的句柄 此处会出错

	// DwmUnregisterThumbnail(thumbnail);
	return thumbnail;
}
