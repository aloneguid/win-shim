#include "util.h"
#include <string>
#include "windows.h"
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <fstream>
//#include "imagehlp.h"

using namespace std;

//#pragma comment(lib, "Imagehlp.lib")

// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#windows-subsystem
// already in imagehlp.h
//#define IMAGE_SUBSYSTEM_WINDOWS_GUI 0x2
//#define IMAGE_SUBSYSTEM_WINDOWS_CUI 0x3

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

std::wstring get_win32_last_error()
{
	DWORD dw = GetLastError();

	PVOID lpMsgBuf;
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
	return sdw;
}

/// <summary>
///  Throws runtime_error with win32 error code and human readable message. Intended to be the last thing the program does.
/// </summary>
void throw_win32_le(const std::wstring& action)
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
	ss << L"Failed";
	if (!action.empty())
	{
		ss << L" to ";
		ss << action;
	}
	ss << ". Code: " << dw << ", message: \"" << sdw << "\".";

	std::string utf8 = wstr_to_str(ss.str());
	throw std::runtime_error(utf8);
}

bool is_console_exe(const std::wstring& image_path)
{
	// if image path is something like "notepad.exe" SHGetFileInfoW won't find it as it assumes
	// relative paths are from current directory, therefore search for it
	TCHAR buf[MAX_PATH];
	::SearchPath(nullptr, image_path.c_str(), nullptr, MAX_PATH, buf, nullptr);

	SHFILEINFOW sfi = { 0 };
	DWORD_PTR ret = ::SHGetFileInfoW(buf, 0, &sfi, sizeof(sfi), SHGFI_EXETYPE);

	return !HIWORD(ret);
}

std::wstring sys_path_search(const std::wstring& image_path)
{
	TCHAR buf[MAX_PATH];
	::SearchPath(nullptr, image_path.c_str(), nullptr, MAX_PATH, buf, nullptr);
	return wstring(buf);

}

void patch_exe_subsystem(const std::wstring& image_path, bool gui)
{
	//idea taken from: https://stackoverflow.com/questions/2435816/how-do-i-poke-the-flag-in-a-win32-pe-that-controls-console-window-display/14806704
	// hacky way to flip subsystem flag to GUI/CUI

	fstream bf(image_path, ios::in | ios::out | ios::binary);
	if (bf)
	{
		bf.seekp(0x3c, ios_base::beg);
		unsigned short header_offset{ 0 };
		bf.read(reinterpret_cast<char*>(&header_offset), sizeof(unsigned short));

		bf.seekp(header_offset, ios_base::beg);
		unsigned int signature{ 0 };
		bf.read(reinterpret_cast<char*>(&signature), sizeof(unsigned short));
		if (signature == 0x4550)
		{
			bf.seekp(static_cast<size_t>(header_offset) + 0x5c);

			unsigned short system =  gui ? IMAGE_SUBSYSTEM_WINDOWS_GUI : IMAGE_SUBSYSTEM_WINDOWS_CUI;
			bf.write(reinterpret_cast<char*>(&system), sizeof(unsigned short));

			//bf.read(reinterpret_cast<char*>(&header_offset), sizeof(unsigned short));
			//cout << system << endl;
			bf.flush();
			bf.close();
		}
	}

}
