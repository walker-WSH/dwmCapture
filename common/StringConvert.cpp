#include "pch.h"
#include <Windows.h>
#include <memory>
#include "StringConvert.h"

namespace str {
//----------------------------------------------
std::wstring a2w(const char *str)
{
	if (!str)
		return L"";

	auto n = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	std::shared_ptr<wchar_t> pBuffer(new wchar_t[(size_t)n + 1],
					 [](wchar_t *p) { delete[] p; });

	n = MultiByteToWideChar(CP_ACP, 0, str, -1, pBuffer.get(), n);
	pBuffer.get()[n] = 0;

	return std::wstring(pBuffer.get());
}

std::string w2a(const wchar_t *str)
{
	if (!str)
		return "";

	auto n = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	std::shared_ptr<char> pBuffer(new char[(size_t)n + 1], [](char *p) { delete[] p; });

	n = WideCharToMultiByte(CP_ACP, 0, str, -1, pBuffer.get(), n, NULL, NULL);
	pBuffer.get()[n] = 0;

	return std::string(pBuffer.get());
}

std::string w2u(const wchar_t *str)
{
	if (!str)
		return "";

	auto n = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	std::shared_ptr<char> pBuffer(new char[(size_t)n + 1], [](char *p) { delete[] p; });

	n = WideCharToMultiByte(CP_UTF8, 0, str, -1, pBuffer.get(), n, NULL, NULL);
	pBuffer.get()[n] = 0;

	return std::string(pBuffer.get());
}

std::wstring u2w(const char *str)
{
	if (!str)
		return L"";

	auto n = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	std::shared_ptr<wchar_t> pBuffer(new wchar_t[(size_t)n + 1],
					 [](wchar_t *p) { delete[] p; });

	n = MultiByteToWideChar(CP_UTF8, 0, str, -1, pBuffer.get(), n);
	pBuffer.get()[n] = 0;

	return std::wstring(pBuffer.get());
}

std::_tstring a2T(const char *str)
{
#ifdef UNICODE
	return a2w(str);
#else
	return str;
#endif
}

std::_tstring w2T(const wchar_t *str)
{
#ifdef UNICODE
	return str;
#else
	return w2a(str);
#endif
}

std::string T2a(const TCHAR *str)
{
#ifdef UNICODE
	return w2a(str);
#else
	return str;
#endif
}

std::wstring T2w(const TCHAR *str)
{
#ifdef UNICODE
	return str;
#else
	return a2w(str);
#endif
}

std::string a2u(const char *str)
{
	return w2u(a2w(str).c_str());
}

std::string u2a(const char *str)
{
	return w2a(u2w(str).c_str());
}

std::string T2u(const TCHAR *str)
{
#ifdef UNICODE
	return w2u(str);
#else
	return w2u(a2w(str).c_str());
#endif
}

std::_tstring u2T(const char *str)
{
#ifdef UNICODE
	return u2w(str);
#else
	return w2a(u2w(str).c_str());
#endif
}
} // namespace ------------------------------
