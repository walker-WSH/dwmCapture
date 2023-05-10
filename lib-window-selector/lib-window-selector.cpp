#include "pch.h"
#include "lib-window-selector.h"
#include "window-selector-impl.h"
#include "FileHelper.h"
#include "HandleWrapper.h"
#include "window-selector-IPC.hpp"
#include <string>
#include <assert.h>
#include <shlwapi.h>
#include <vector>
#include <process.h>
#include <Windows.h>
#include <memory>

namespace selector {

std::shared_ptr<IWindowSelector> createSelector()
{
	return std::make_shared<WindowSelector>();
}

}; //namespace selector
