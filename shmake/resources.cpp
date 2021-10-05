#include "resources.h"
#include <iostream>
#include "util.h"
#include <fstream>
#include <boost/algorithm/string.hpp>

#define MAX_LOADSTRING 1024

#pragma comment(lib, "version.lib")

using namespace std;

// this struct in not defined in any system header files, but is mentioned here:
// https://docs.microsoft.com/en-us/windows/win32/menurc/vs-versioninfo
// note the remarks:
//
// This structure is not a true C-language structure because it contains variable-length members.
// This structure was created solely to depict the organization of data in a version resource and does not appear in any of the
// header files shipped with the Windows Software Development Kit (SDK).
typedef struct {
	WORD             wLength;
	// The length, in bytes, of the VS_VERSIONINFO structure. This length does not include any padding that aligns any subsequent version resource data on a 32-bit boundary.
	WORD             wValueLength;
	// The type of data in the version resource. This member is 1 if the version resource contains text data and 0 if the version resource contains binary data.
	WORD             wType;
	// The Unicode string L"VS_VERSION_INFO".
	WCHAR            szKey;
	WORD             Padding1;
	VS_FIXEDFILEINFO Value;
	WORD             Padding2;
	// An array of zero or one StringFileInfo structures, and zero or one VarFileInfo structures that are children of the current VS_VERSIONINFO structure.
	WORD             Children;
} VS_VERSIONINFO;

BOOL EnumResourceNamesFuncFindFirst(HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR lParam)
{
	LPWSTR* ret = (LPWSTR*)lParam;
	*ret = lpName;
	return FALSE;
}

BOOL EnumResourceLanguagesFindFirst(HMODULE hModule, LPCWSTR lpType, LPCWSTR lpName, WORD wLanguage, LONG_PTR lParam)
{
	WORD* ret = (WORD*)lParam;
	*ret = wLanguage;
	return FALSE;
}

resources::resources(const std::wstring& file_path)
	: file_path { file_path }, edit_made { false }, hEdit { 0 }
{
	if (file_path.empty())
	{
		hInstance = ::GetModuleHandle(NULL);
	}
	else
	{
		hInstance = ::LoadLibraryEx(file_path.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);

		if (!hInstance)
			throw_win32_le();
	}
}

resources::~resources()
{
	//commit_changes();
}

void resources::commit_changes()
{
	if (edit_made)
	{
		if (!::EndUpdateResource(hEdit, false))
			throw_win32_le(L"Committing resource");

		edit_made = false;
	}
}

std::wstring resources::load_string(UINT id)
{
	WCHAR szs[MAX_LOADSTRING];
	::LoadString(hInstance, id, szs, MAX_LOADSTRING);
	return wstring(szs);
}

std::wstring resources::get_file_version()
{
	// https://www.codeproject.com/Articles/8628/Retrieving-version-information-from-your-local-app

	wstring result;

	HRSRC hVersion = ::FindResource(hInstance, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
	if (hVersion)
	{
		HGLOBAL hGlobal = ::LoadResource(hInstance, hVersion);
		if (hGlobal)
		{
			LPVOID versionInfo = ::LockResource(hGlobal);
			if (versionInfo)
			{
				LPVOID lpBuffer;
				UINT puLen;
				if (::VerQueryValue(versionInfo, L"\\VarFileInfo\\Translation", &lpBuffer, &puLen))
				{
					if (puLen == 4)
					{
						DWORD lang;
						memcpy(&lang, lpBuffer, 4);
						wchar_t sbuf[1024];
						wsprintf(sbuf, L"\\StringFileInfo\\%02X%02X%02X%02X\\FileVersion",
							(lang & 0xff00) >> 8, lang & 0xff, (lang & 0xff000000) >> 24,
							(lang & 0xff0000) >> 16);
						
						if (::VerQueryValue(versionInfo, sbuf, &lpBuffer, &puLen))
						{
							result = reinterpret_cast<wchar_t*>(lpBuffer);
						}
					}
				}
			}

			::FreeResource(hGlobal);
		}
	}

	size_t last_dot_idx = result.find_last_of('.');
	if (last_dot_idx != wstring::npos)
	{
		result.erase(last_dot_idx);
	}

	return result;
}

void resources::set_main_icon(const std::wstring& path)
{
	HMODULE hSrcModule = ::LoadLibraryEx(path.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);
	HRSRC hSrcIcon = ::FindResource(hSrcModule, MAKEINTRESOURCE(1), RT_ICON);

	//::FreeLibrary(hSrcModule);
}

void resources::replace_string_table(int index, const std::vector<std::wstring>& strings)
{
	open_for_edit();

	vector<WORD> buffer;
	buffer.push_back(static_cast<WORD>(0));
	for (const wstring& w : strings)
	{
		buffer.push_back(static_cast<WORD>(w.size()));
		std::copy(w.begin(), w.end(), std::back_inserter(buffer));
	}

	// You have to replace the entire string table, not just a single entry. Good performance wise.
	if (!::UpdateResource(hEdit,
		RT_STRING,
		MAKEINTRESOURCE(index),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		reinterpret_cast<void*>(&buffer[0]), buffer.size() * sizeof(WORD)))
	{
		throw_win32_le();
	}
}

void resources::extract_binary_to_file(int res_id, const std::wstring& path)
{
	// note that it is not necessary to free/unload any handles here

	HRSRC hResInfo = ::FindResource(hInstance, MAKEINTRESOURCE(res_id), RT_RCDATA);
	if (!hResInfo) throw_win32_le(L"find shim inside this module");

	HGLOBAL hResData = ::LoadResource(hInstance, hResInfo);
	if (!hResData) throw_win32_le(L"load shim resource");

	size_t size = ::SizeofResource(hInstance, hResInfo);
	LPVOID data = ::LockResource(hResData);

	// write it out
	ofstream f(path, ios::out | ios::binary);
	f.write((char*)data, size);
	f.close();
}

void resources::replace_version_info(const resources& other)
{
	raw_copy(other, RT_VERSION);
}

void resources::replace_icon(const resources& other)
{
	raw_copy(other, RT_ICON);
}

void resources::open_for_edit()
{
	if (edit_made) return;

	if (hEdit = ::BeginUpdateResource(file_path.c_str(), false))
	{
		edit_made = true;
	}
	else
	{
		DWORD err = ::GetLastError();
	}
}

bool resources::open_first_resource(LPCWSTR lpType, LPWSTR* lpName, WORD* wLanguage, DWORD* dataSize, LPVOID* data) const
{
	::EnumResourceNames(hInstance, lpType, EnumResourceNamesFuncFindFirst, (LONG_PTR)lpName);
	if (*lpName)
	{
		::EnumResourceLanguages(hInstance, lpType, *lpName, EnumResourceLanguagesFindFirst, (LONG_PTR)wLanguage);
		if (*wLanguage)
		{
			if (data)
			{
				HRSRC hResInfo = ::FindResourceEx(hInstance, lpType, *lpName, *wLanguage);
				if (hResInfo)
				{
					*dataSize = ::SizeofResource(hInstance, hResInfo);
					if (*dataSize)
					{
						HGLOBAL hResData = ::LoadResource(hInstance, hResInfo);
						if (hResData)
						{
							*data = ::LockResource(hResData);
							return true;
						}
					}
				}
			}
			else return true;
		}
	}
	return false;
}

bool resources::raw_copy(const resources& other, LPCWSTR lpType)
{
	// get first RT_VERSION and first language
	LPWSTR lpName, lpNameOther;
	WORD wLanguage, wLanguageOther;
	DWORD dataSize{ 0 };
	LPVOID data { 0 };
	if (other.open_first_resource(lpType, &lpNameOther, &wLanguageOther, &dataSize, &data) &&
		open_first_resource(lpType, &lpName, &wLanguage, nullptr, nullptr))
	{
		open_for_edit();

		if (!::UpdateResource(hEdit, lpType, lpName, wLanguage, data, dataSize))
		{
			throw_win32_le();
		}

		return true;
	}
	return false;
}
