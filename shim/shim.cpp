#include <iostream>
#include <string>
#include "windows.h"
#include "resource.h"
#include "../shmake/resources.h"
#include "../shmake/util.h"

using namespace std;

const wstring CMD_TOKEN{ L"%s" };

void get_caps(const wstring& caps_str, bool& clipboard, bool& no_kill)
{
    auto detect_cap = [&clipboard, &no_kill](const wstring& cap_name)
    {
        if (cap_name == L"clipboard")
            clipboard = true;
        else if (cap_name == L"no-kill")
            no_kill = true;
    };

    wstring cap_name;
    for (wchar_t c : caps_str)
    {
        if (c == L';')
        {
            detect_cap(cap_name);
            cap_name.clear();
        }
        else
        {
            cap_name.push_back(c);
        }
    }
    detect_cap(cap_name);
}

// entry point has to be wWinMain (Unicode version of WinMain) instead of "main" - this allows us to change
// target subsystem to "windows" and turn off console allocation.
// note that "shmake" is still standard console executable.
int wmain(int argc, wchar_t* argv[], wchar_t *envp[])
{
    resources res;
    auto image_path = res.load_string(IDS_IMAGE_PATH);
    auto args_pattern = res.load_string(IDS_ARGS);
    auto caps_str = res.load_string(IDS_CAPABILITIES);

    bool cap_clipboard = false;
    bool cap_no_kill = false;
    get_caps(caps_str, cap_clipboard, cap_no_kill);

    // args
    auto args = args_pattern;

    wstring passed_arg;
    for (int i = 1; i < argc; i++)
    {
        if (i > 1) passed_arg += L" ";
        passed_arg += argv[i];
    }

    // do token replacement regardless of whether we have an argument or not - if we do, it needs to be deleted anyway
    size_t pos = args.find(CMD_TOKEN);
    if (pos != string::npos)
    {
        args.replace(pos, CMD_TOKEN.size(), passed_arg);
    }

    wstring full_cmd = image_path;
    if (!args.empty())
    {
        full_cmd += L" " + args;
    }

    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    si.cb = sizeof(si);

    //wcout << L"launching " << full_cmd << endl;

    if(!::CreateProcess(
        NULL, // lpApplicationName
        const_cast<LPWSTR>(full_cmd.c_str()),
        NULL, // lpProcessAttributes
        NULL, // lpThreadAttributes
        TRUE, // bInheritHandles
        // create process in suspended state in case we need to do some adjustments before it executes
        CREATE_SUSPENDED, // dwCreationFlags
        NULL, // lpEnvironment
        NULL, // lpCurrentDirectory
        &si,
        &pi))
    {
        //wcout << L"could not create process - " << get_win32_last_error() << endl;
        return 1;
    }
    else
    {
        HANDLE hJob = ::CreateJobObject(
            NULL,
            NULL // job name, not needed
        );

        // perfect place to create something like a job object - anything that happens just before process starts.
        ::AssignProcessToJobObject(hJob, pi.hProcess);

        JOBOBJECT_BASIC_UI_RESTRICTIONS jUI = { 0 };
        jUI.UIRestrictionsClass = JOB_OBJECT_UILIMIT_DESKTOP |
            JOB_OBJECT_UILIMIT_DISPLAYSETTINGS |
            JOB_OBJECT_UILIMIT_EXITWINDOWS |
            JOB_OBJECT_UILIMIT_GLOBALATOMS |
            JOB_OBJECT_UILIMIT_HANDLES |
            JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS;
        if (!cap_clipboard)
        {
            jUI.UIRestrictionsClass |= JOB_OBJECT_UILIMIT_READCLIPBOARD | JOB_OBJECT_UILIMIT_WRITECLIPBOARD;
        }
        // can limit clipboard access above if needed!
        if (!::SetInformationJobObject(hJob, JobObjectBasicUIRestrictions, &jUI, sizeof(jUI)))
        {
            wstring emsg = get_win32_last_error();
            wcout << L"failed to set UI limits: " << emsg << endl;
        }

        if (!cap_no_kill)
        {
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION jEli = { 0 };
            jEli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            if (!::SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jEli, sizeof(jEli)))
            {
                wstring emsg = get_win32_last_error();
                //wcout << L"failed to set extended limits: " << emsg << endl;
            }
        }

        JOBOBJECT_CPU_RATE_CONTROL_INFORMATION jCpu = { 0 };

        // now the process starts for real
        //wcout << L"resuming" << endl;
        ::ResumeThread(pi.hThread);

		::WaitForSingleObject(pi.hProcess, INFINITE);

		DWORD exit_code = 0;
		// if next line fails, code is still 0
		::GetExitCodeProcess(pi.hProcess, &exit_code);
        //wcout << L"exited with code " << exit_code << endl;

		// free OS resources
		::CloseHandle(pi.hProcess);
		::CloseHandle(pi.hThread);
        ::CloseHandle(hJob); // required to apply termination job limits

		return exit_code;
    }
}