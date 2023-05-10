#include "pch.h"
#include "FileHelper.h"
#include <shlobj.h>

#pragma comment(lib, "Shlwapi.lib")

namespace fileHelper {

bool IsFileExist(const wchar_t *unicode_path)
{
	if (!unicode_path)
		return false;
	else
		return ((_waccess(unicode_path, 0)) != -1);
}

std::wstring GetFileFullPath(HINSTANCE hInstance)
{
	WCHAR szFilePath[MAX_PATH] = {};
	GetModuleFileNameW(hInstance, szFilePath, MAX_PATH);
	return szFilePath;
}

std::wstring GetFileDirectory(HINSTANCE hInstance)
{
	wchar_t dir[MAX_PATH] = {};
	GetModuleFileNameW(hInstance, dir, MAX_PATH);
	PathRemoveFileSpecW(dir);
	return std::wstring(dir) + std::wstring(L"\\");
}

std::string ExtractFileName(const char *full_path)
{
	if (!full_path)
		return "";

	LPCSTR name = PathFindFileNameA(full_path);
	if (!name)
		return "";

	return name;
}

std::string ExtractExtension(const char *full_path)
{
	if (!full_path)
		return "";

	auto ret = PathFindExtensionA(full_path);
	if (!ret)
		return "";

	return ret;
}

std::wstring GetDesktopDirectory()
{
	wchar_t dir[_MAX_PATH] = {}; // without '\' at the end
	SHGetSpecialFolderPathW(NULL, dir, CSIDL_DESKTOP, 0);

	return std::wstring(dir) + std::wstring(L"\\");
}

}; // namespace fileHelper
