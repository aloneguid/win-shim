#include "resources.h"
#include <iostream>
#include "util.hpp"

#define MAX_LOADSTRING 1024

using namespace std;

resources::resources(const std::string& file_path)
	: file_path { file_path }
{
	if (file_path.empty())
	{
		hInstance = ::GetModuleHandle(nullptr);
	}
	else
	{
		if (!::LoadLibraryExA(file_path.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE))
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
	return std::wstring(szs);
}

void resources::copy_main_icon(const resources& source)
{
	HICON hSrcIcon = ::ExtractIconA(hInstance, file_path.c_str(), 0);

	//::UpdateResource()

	::DestroyIcon(hSrcIcon);
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

	if (!::UpdateResource(hEdit,
		RT_STRING,
		MAKEINTRESOURCE(index),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		reinterpret_cast<void*>(&buffer[0]), buffer.size() * sizeof(WORD)))
	{
		throw_win32_le();
	}
}

void resources::open_for_edit()
{
	if (edit_made) return;

	if (hEdit = ::BeginUpdateResourceA(file_path.c_str(), false))
	{
		edit_made = true;
	}
	else
	{
		DWORD err = ::GetLastError();
	}
}
