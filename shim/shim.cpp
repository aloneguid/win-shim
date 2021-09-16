#include <iostream>
#include <string>
#include "windows.h"
#include "resource.h"
#include "../shmake/resources.h"
#include "../shmake/util.h"

using namespace std;

const wstring CMD_TOKEN = L"%s";

int wmain(int argc, wchar_t* argv[])
{
    resources res;
    auto cmd_pattern = res.load_string(IDS_TARGET_CMDLINE);

    wcout << "cmdline: " << cmd_pattern << endl;

    wstring cmd = cmd_pattern;

    if (argc > 1)
    {
        size_t pos = cmd.find(CMD_TOKEN);
        if (pos != string::npos)
        {
            cmd.replace(pos, CMD_TOKEN.size(), argv[1]);
        }
    }

    wcout << "final: " << cmd << endl;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ::ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    ::ZeroMemory(&pi, sizeof(pi));

    if(!::CreateProcess(
        NULL, // lpApplicationName
        const_cast<LPWSTR>(cmd.c_str()),
        NULL, // lpProcessAttributes
        NULL, // lpThreadAttributes
        TRUE, // bInheritHandles
        NULL, // dwCreationFlags
        NULL, // lpEnvironment
        NULL, // lpCurrentDirectory
        &si,
        &pi))
    {
        wstring emsg = get_win32_last_error();
        wcout << L"failed: " << emsg;
        return 1;
    }

    wcout << L"waiting... ";
	::WaitForSingleObject(pi.hProcess, INFINITE);
    wcout << L"done." << endl;

    DWORD exit_code = 0;
    // if next line fails, code is still 0
    ::GetExitCodeProcess(pi.hProcess, &exit_code);
    
    // free OS resources
    ::CloseHandle(pi.hProcess);
    ::CloseHandle(pi.hThread);

    return exit_code;
}