#include <iostream>
#include <string>
#include "windows.h"
#include "resource.h"
#include "../shmake/resources.h"
#include "../shmake/util.h"

using namespace std;

const wstring CMD_TOKEN{ L"%s" };

// entry point has to be wWinMain (unicode version of WinMain) instead of "main" - this allows us to change
// target subsystem to "windows" and turn off console allocation.
// note that "shmake" is still standard console executable.
int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR pCmdLine,
    _In_ int nCmdShow)
{
    resources res;
    auto cmd_pattern = res.load_string(IDS_TARGET_CMDLINE);

    wcout << "cmdline: " << cmd_pattern << endl;

    wstring cmd = cmd_pattern;
    wstring arg(pCmdLine);

    if (!arg.empty())
    {
        size_t pos = cmd.find(CMD_TOKEN);
        if (pos != string::npos)
        {
            cmd.replace(pos, CMD_TOKEN.size(), arg);
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

    DWORD exit_code = 0;
    // if next line fails, code is still 0
    ::GetExitCodeProcess(pi.hProcess, &exit_code);
	wcout << L"done (code: " << exit_code << L")" << endl;

    
    // free OS resources
    ::CloseHandle(pi.hProcess);
    ::CloseHandle(pi.hThread);

    return exit_code;
}