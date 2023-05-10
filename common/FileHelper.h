#pragma once
#include <string>
#include <TCHAR.H>
#include <Windows.h>
#include <shlwapi.h>

namespace fileHelper {

bool IsFileExist(const wchar_t *unicode_path);

std::wstring GetFileFullPath(HINSTANCE hInstance = NULL);
std::wstring GetFileDirectory(HINSTANCE hInstance = NULL); // include "\"

std::string ExtractFileName(const char *full_path);
std::string ExtractExtension(const char *full_path);

std::wstring GetDesktopDirectory(); // with '\' at the end

}; // namespace fileHelper
