#pragma once

#include <string>
#include "windows.h"
#include <stdexcept>
#include <sstream>

std::wstring str_to_wstr(const std::string& str)
{
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

std::string wstr_to_str(const std::wstring& wstr)
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
	return strTo;
}

/// <summary>
///  Throws runtime_error with win32 error code and human readable message. Intended to be the last thing the program does.
/// </summary>
void throw_win32_le(const std::wstring& function_name = L"")
{
	// https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
	DWORD dw = GetLastError();

	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;

	::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);
	std::wstring sdw((wchar_t*)lpMsgBuf);
	::LocalFree(lpMsgBuf);

	// make it nice

	std::wostringstream ss;
	if (!function_name.empty())
	{
		ss << L"\"" << function_name << "\" failed.";
	}
	else
	{
		ss << "Failed.";
	}
	ss << " Code: " << dw << ", message: \"" << sdw << "\".";
	
	std::string utf8 = wstr_to_str(ss.str());
	throw std::runtime_error(utf8);
}