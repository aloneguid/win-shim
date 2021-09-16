#pragma once

#include <string>
#include "windows.h"

std::wstring str_to_wstr(const std::string& str);

std::string wstr_to_str(const std::wstring& wstr);

std::wstring get_win32_last_error();

/// <summary>
///  Throws runtime_error with win32 error code and human readable message. Intended to be the last thing the program does.
/// </summary>
void throw_win32_le(const std::wstring& function_name = L"");