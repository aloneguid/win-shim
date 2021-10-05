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

	// returns file version resource value, formatted for display
	std::wstring get_file_version();

	void set_main_icon(const std::wstring& path);

	void replace_string_table(int index, const std::vector<std::wstring>& strings);

	void extract_binary_to_file(int res_id, const std::wstring& path);

	void replace_version_info(const resources& other);

	void replace_icon(const resources& other);

	void commit_changes();

private:
	std::wstring file_path;
	HINSTANCE hInstance;
	bool edit_made;
	HANDLE hEdit;

	void open_for_edit();
	bool open_first_resource(LPCWSTR lpType, LPWSTR* lpName, WORD* wLanguage, DWORD* dataSize, LPVOID* data) const;
	bool raw_copy(const resources& other, LPCWSTR lpType);
};

