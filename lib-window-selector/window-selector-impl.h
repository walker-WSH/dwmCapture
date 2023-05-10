#pragma once
#include <Windows.h>
#include <functional>
#include <string>
#include <assert.h>
#include <shlwapi.h>
#include <vector>
#include <process.h>
#include <Windows.h>
#include <memory>
#include "lib-window-selector.h"

namespace selector {

class WindowSelector;
struct ThreadParams {
	WindowSelector *self;
	std::wstring dir;
	HWND centerWith;
	SIZE resolution;
	std::weak_ptr<IWindowSelectorEvent> callback;
};

class WindowSelector : public IWindowSelector {
public:
	WindowSelector();
	virtual ~WindowSelector();

	virtual bool selectWindow(std::weak_ptr<IWindowSelectorEvent> cb, HWND centerWith,
				  SIZE resolution = {0, 0});

protected:
	static unsigned __stdcall ThreadFunc(void *pParam);
	unsigned ThreadFuncInner(std::shared_ptr<ThreadParams> info);
	std::string GenerateGuid();

private:
	HANDLE m_hExitEvent = 0;
	std::vector<HANDLE> m_vecThreads;
};

}; //namespace selector
