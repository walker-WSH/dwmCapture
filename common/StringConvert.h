#pragma once
#include <string>
#include <TCHAR.H>

//-----------------------------------------------------
#define _CRT_NON_CONFORMING_SWPRINTFS

#ifdef UNICODE
#define _tstring wstring
#define _ttoupper towupper
#define _ttolower tolower
#else
#define _tstring string
#define _ttoupper toupper
#define _ttolower towlower
#endif

namespace str {

std::_tstring a2T(const char *str);
std::wstring a2w(const char *str);
std::string a2u(const char *str);

std::_tstring w2T(const wchar_t *str);
std::string w2a(const wchar_t *str);
std::string w2u(const wchar_t *str);

std::_tstring u2T(const char *str);
std::wstring u2w(const char *str);
std::string u2a(const char *str);

std::string T2a(const TCHAR *str);
std::string T2u(const TCHAR *str);
std::wstring T2w(const TCHAR *str);

} // namespace str
