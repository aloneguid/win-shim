#pragma once
#include <windows.h>
#include <string>
#include <vector>

// for cool stuff see https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-beginupdateresourcea

// good samples here https://github.com/Squirrel/Squirrel.Windows/blob/develop/src/WriteZipToSetup/WriteZipToSetup.cpp

class resources
{
public:
	resources(const std::wstring& file_path = L"");
	~resources();

	std::wstring load_string(UINT id);

	void copy_main_icon(const resources& source);

	void replace_string_table(int index, const std::vector<std::wstring>& strings);

	void extract_binary_to_file(int res_id, const std::wstring& path);

	void commit_changes();

private:
	std::wstring file_path;
	HINSTANCE hInstance;
	bool edit_made;
	HANDLE hEdit;

	void open_for_edit();
};

