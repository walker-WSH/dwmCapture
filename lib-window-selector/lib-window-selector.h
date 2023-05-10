#pragma once
#include <Windows.h>

#define SELECTOR_EXPORT __declspec(dllexport)

namespace selector {

SELECTOR_EXPORT HWND selectWindow(POINT centerPos, SIZE resolution = {0, 0});

}; //namespace selector
