#pragma once
#include <Windows.h>
#include <string>

#define ALIGN(bytes, align) (((bytes) + ((align)-1)) & ~((align)-1))

static const auto SELECTOR_APP = L"window-selector.exe";
static const auto SELECTOR_TOOL_CMD = "run_as_tool";

static const auto SELECTOR_ALIVE_EVENT = "selector_alive";
static const auto SELECTOR_INFO_MAP = "selector_info";

static const auto SELECTOR_EXIT_CODE_OK = 0;
static const auto SELECTOR_EXIT_CODE_CANCEL = 1;
static const auto SELECTOR_EXIT_CODE_ERROR = 2;

struct WindowSelectorIPC {
	POINT centerPos;
	SIZE resolution;

	DWORD64 outputWnd = 0; // should not use HWND
};
