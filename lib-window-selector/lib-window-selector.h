#pragma once
#include <memory>
#include <Windows.h>
#include <functional>

namespace selector {

class IWindowSelectorEvent {
public:
	virtual ~IWindowSelectorEvent() = default;
	virtual void onSelectResult(HWND hWnd) = 0;
};

class IWindowSelector {
public:
	virtual ~IWindowSelector() = default;
	virtual bool selectWindow(std::weak_ptr<IWindowSelectorEvent> cb, HWND centerWith,
				  SIZE resolution = {0, 0}) = 0;
};

__declspec(dllexport) std::shared_ptr<IWindowSelector> createSelector();

}; //namespace selector
