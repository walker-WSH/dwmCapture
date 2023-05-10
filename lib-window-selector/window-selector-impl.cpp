#include "pch.h"
#include "window-selector-impl.h"
#include "window-selector-IPC.hpp"
#include "FileHelper.h"
#include "HandleWrapper.h"

#pragma comment(lib, "Shlwapi.lib")

extern HMODULE g_hDllModule;

namespace selector {

WindowSelector::WindowSelector()
{
	m_hExitEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
}

WindowSelector::~WindowSelector()
{
	SetEvent(m_hExitEvent);
	for (auto &item : m_vecThreads) {
		WaitForSingleObject(item, INFINITE);
		CHandleWrapper::CloseHandleEx(item);
	}

	m_vecThreads.clear();
	CHandleWrapper::CloseHandleEx(m_hExitEvent);
}

bool WindowSelector::selectWindow(std::weak_ptr<IWindowSelectorEvent> cb, HWND centerWith,
				  SIZE resolution)
{
	bool selecting = false;
	auto itr = m_vecThreads.begin();
	while (itr != m_vecThreads.end()) {
		if (CHandleWrapper::IsHandleSigned(*itr)) {
			CHandleWrapper::CloseHandleEx(*itr);
			itr = m_vecThreads.erase(itr);
			continue;
		}
		selecting = true;
		++itr;
	}

	if (selecting) {
		assert(false && "being selecting");
		return false;
	}

	std::wstring dir = fileHelper::GetFileDirectory(g_hDllModule);
	dir += SELECTOR_APP;

	if (!fileHelper::IsFileExist(dir.c_str())) {
		assert(false && "window-selector.exe not found");
		return false;
	}

	ThreadParams *params = new ThreadParams();
	params->dir = dir;
	params->self = this;
	params->centerWith = centerWith;
	params->resolution = resolution;
	params->callback = cb;

	HANDLE hThread = (HANDLE)_beginthreadex(0, 0, ThreadFunc, params, 0, 0);
	m_vecThreads.push_back(hThread);

	return true;
}

unsigned __stdcall WindowSelector::ThreadFunc(void *pParam)
{
	auto params = (ThreadParams *)pParam;
	std::shared_ptr<ThreadParams> temp(params);
	return temp->self->ThreadFuncInner(temp);
}

unsigned WindowSelector::ThreadFuncInner(std::shared_ptr<ThreadParams> info)
{
	bool canceled = false;
	HWND selectedWnd = 0;

	HANDLE hProcessHandle = 0;
	HANDLE hAliveSignal = 0;
	HANDLE hIPCMapHandle = 0;
	void *pIPCMapViewOfFile = nullptr;
	WindowSelectorIPC *pIPCMapInfo = nullptr;

	do {
		//----------------------------------------------------------------------------------
		std::string guid = GenerateGuid();
		std::string aliveName = guid + std::string(SELECTOR_ALIVE_EVENT);
		hAliveSignal = CHandleWrapper::GetEvent(aliveName.c_str(), true);
		if (!CHandleWrapper::IsHandleValid(hAliveSignal)) {
			assert(false);
			break;
		}

		SIZE_T size = ALIGN(sizeof(WindowSelectorIPC), 64);
		std::string infoName = guid + std::string(SELECTOR_INFO_MAP);

		bool bNewCreate = false;
		hIPCMapHandle =
			CHandleWrapper::GetMap(infoName.c_str(), (unsigned)size, &bNewCreate);
		if (!CHandleWrapper::IsHandleValid(hIPCMapHandle) || !bNewCreate) {
			assert(false);
			break;
		}

		pIPCMapViewOfFile = MapViewOfFile(hIPCMapHandle, FILE_MAP_ALL_ACCESS, 0, 0, size);
		if (!pIPCMapViewOfFile) {
			assert(false);
			break;
		}

		POINT center = {0, 0};
		if (IsWindow(info->centerWith)) {
			RECT rc;
			GetWindowRect(info->centerWith, &rc);

			center.x = rc.left + (rc.right - rc.left) / 2;
			center.y = rc.top + (rc.bottom - rc.top) / 2;
		}

		memset(pIPCMapViewOfFile, 0, size);
		pIPCMapInfo = (WindowSelectorIPC *)pIPCMapViewOfFile;
		pIPCMapInfo->centerPos = center;
		pIPCMapInfo->resolution = info->resolution;
		pIPCMapInfo->parentPID = GetCurrentProcessId();

		//----------------------------------------------------------------------------------
		wchar_t cmd[2048];
		swprintf_s(cmd, 2048, L"\"%s\" \"%hs\" \"%hs\"", info->dir.c_str(),
			   SELECTOR_TOOL_CMD, guid.c_str());

		PROCESS_INFORMATION pi = {};
		STARTUPINFO si = {};
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_FORCEOFFFEEDBACK;
		si.wShowWindow = SW_HIDE;

		BOOL bOK = CreateProcessW((LPWSTR)info->dir.c_str(), cmd, NULL, NULL, TRUE,
					  CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
		if (!bOK) {
			assert(false);
			break;
		}

		CloseHandle(pi.hThread);
		hProcessHandle = pi.hProcess;

		// wait selector process to return value
		HANDLE events[] = {m_hExitEvent, hProcessHandle};
		auto count = ARRAYSIZE(events);
		while (TRUE) {
			WaitForMultipleObjects((DWORD)count, events, FALSE, 10);

			if (CHandleWrapper::IsHandleSigned(hProcessHandle, 0)) {
				DWORD code = 0;
				GetExitCodeProcess(hProcessHandle, &code);
				if (code == SELECTOR_EXIT_CODE_OK) {
					selectedWnd = (HWND)pIPCMapInfo->outputWnd;
				}

				break;
			}

			if (info->callback.expired() ||
			    CHandleWrapper::IsHandleSigned(m_hExitEvent, 0)) {
				TerminateProcess(hProcessHandle, SELECTOR_EXIT_CODE_CANCEL);
				canceled = true;
				break;
			}
		}

	} while (false);

	if (CHandleWrapper::IsHandleValid(hProcessHandle)) {
		CHandleWrapper::CloseHandleEx(hProcessHandle);
	}

	if (pIPCMapViewOfFile) {
		UnmapViewOfFile(pIPCMapViewOfFile);
	}

	CHandleWrapper::CloseHandleEx(hIPCMapHandle);
	CHandleWrapper::CloseHandleEx(hAliveSignal);

	if (!canceled) {
		auto cb = info->callback.lock();
		if (cb)
			cb->onSelectResult(selectedWnd);
	}

	return 0;
}

std::string WindowSelector::GenerateGuid()
{
	char buf[MAX_PATH] = {0};

	GUID guid;
	HRESULT hr = CoCreateGuid(&guid);
	if (FAILED(hr)) {
		snprintf(buf, MAX_PATH, "%u_%llu", GetCurrentThreadId(), GetTickCount64());
	} else {
		snprintf(buf, MAX_PATH, "{%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X}",
			 guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],
			 guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6],
			 guid.Data4[7]);
	}

	return std::string(buf);
}

}; //namespace selector
