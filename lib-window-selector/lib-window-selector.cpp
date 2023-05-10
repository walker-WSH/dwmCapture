#include "pch.h"
#include "lib-window-selector.h"
#include "FileHelper.h"
#include "HandleWrapper.h"
#include "window-selector-IPC.hpp"
#include <string>
#include <assert.h>
#include <shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

extern HMODULE g_hDllModule;

namespace selector {

std::string GenerateGuid()
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

SELECTOR_EXPORT HWND selectWindow(POINT centerPos, SIZE resolution)
{
	std::wstring dir = fileHelper::GetFileDirectory(g_hDllModule);
	dir += SELECTOR_APP;

	if (!fileHelper::IsFileExist(dir.c_str())) {
		assert(false && "window-selector.exe not found");
		return 0;
	}

	HWND selectedWnd = 0;

	HANDLE m_hProcessHandle = 0;
	HANDLE m_hAliveSignal = 0;
	HANDLE m_hMapHandle = 0;
	void *m_pMapViewOfFile = nullptr;
	WindowSelectorIPC *m_pMapInfo = nullptr;

	do {
		//----------------------------------------------------------------------------------
		std::string guid = GenerateGuid();
		std::string aliveName = guid + std::string(SELECTOR_ALIVE_EVENT);
		m_hAliveSignal = CHandleWrapper::GetEvent(aliveName.c_str(), true);
		if (!CHandleWrapper::IsHandleValid(m_hAliveSignal)) {
			assert(false);
			break;
		}

		SIZE_T size = ALIGN(sizeof(WindowSelectorIPC), 64);
		std::string infoName = guid + std::string(SELECTOR_INFO_MAP);

		bool bNewCreate = false;
		m_hMapHandle =
			CHandleWrapper::GetMap(infoName.c_str(), (unsigned)size, &bNewCreate);
		if (!CHandleWrapper::IsHandleValid(m_hMapHandle) || !bNewCreate) {
			assert(false);
			break;
		}

		m_pMapViewOfFile = MapViewOfFile(m_hMapHandle, FILE_MAP_ALL_ACCESS, 0, 0, size);
		if (!m_pMapViewOfFile) {
			assert(false);
			break;
		}

		memset(m_pMapViewOfFile, 0, size);
		m_pMapInfo = (WindowSelectorIPC *)m_pMapViewOfFile;
		m_pMapInfo->centerPos = centerPos;
		m_pMapInfo->resolution = resolution;

		//----------------------------------------------------------------------------------
		wchar_t cmd[2048];
		swprintf_s(cmd, 2048, L"\"%s\" \"%hs\" \"%hs\"", dir.c_str(), SELECTOR_TOOL_CMD,
			   guid.c_str());

		PROCESS_INFORMATION pi = {};
		STARTUPINFO si = {};
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_FORCEOFFFEEDBACK;
		si.wShowWindow = SW_HIDE;

		BOOL bOK = CreateProcessW((LPWSTR)dir.c_str(), cmd, NULL, NULL, TRUE,
					  CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
		if (!bOK) {
			assert(false);
			break;
		}

		CloseHandle(pi.hThread);
		m_hProcessHandle = pi.hProcess;

		// wait selector process to return value
		CHandleWrapper::IsHandleSigned(m_hProcessHandle, INFINITE);

		DWORD code = 0;
		GetExitCodeProcess(m_hProcessHandle, &code);
		if (code == SELECTOR_EXIT_CODE_OK) {
			selectedWnd = (HWND)m_pMapInfo->outputWnd;
		}

	} while (false);

	if (CHandleWrapper::IsHandleValid(m_hProcessHandle)) {
		CHandleWrapper::CloseHandleEx(m_hProcessHandle);
	}

	if (m_pMapViewOfFile) {
		UnmapViewOfFile(m_pMapViewOfFile);
	}

	CHandleWrapper::CloseHandleEx(m_hMapHandle);
	CHandleWrapper::CloseHandleEx(m_hAliveSignal);

	return selectedWnd;
}

}; //namespace selector
