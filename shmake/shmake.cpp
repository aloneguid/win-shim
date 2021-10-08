#include <iostream>
#include "boost/program_options.hpp"
#include "boost/algorithm/string/join.hpp"
#include "resource.h"
#include "resources.h"
#include "util.h"
#include <filesystem>

using namespace std;
namespace po = boost::program_options;
namespace fs = std::filesystem;

int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
	wstring version;
	{
		resources this_res;
		version = this_res.get_file_version();
	}
	wcout << L"Windows Shim Maker v" << version << L" by @aloneguid (https://github.com/aloneguid/win-shim)" << endl << endl;

	bool no_mirror = false;
	wstring input;
	wstring app_path;
	wstring output;
	wstring args;
	vector<wstring> caps;

	// tutorial: https://www.boost.org/doc/libs/1_77_0/doc/html/program_options/tutorial.html#id-1.3.32.4.3
	po::options_description desc("allowed options");
	desc.add_options()
		("help,h", "Produce help message.")
		("input,i", po::wvalue<wstring>(&input)->required(), "Path to input executable (required).")
		("output,o", po::wvalue<wstring>(&output)->default_value(L"", ""), "Path to output executable. If not passed, named same as input executable and written to the current directory.")
		("args,a", po::wvalue<wstring>(&args)->default_value(L"", ""),
			"Command line to execute. By default arguments are passed through as is, but you can pass any string and use %s as substitute for the original arguments.")
		("app-path", po::wvalue<wstring>(&app_path)->default_value(L"", ""),
			"App path to use instead of 'input' when launching the shim. This is useful when you need to launch a different program, but still copy version and icon information from the 'input'.")
		("capabilities,c", po::wvalue<vector<wstring>>(&caps)->multitoken(), "Capabilities i.e. what target app is allowed to do. By default clipboard access is allowed. Supported options: clipboard - allow clipboard access, no-kill - do not kill target process when shim exits. You can pass multiple capabilities i.e. -c clipboard no-kill");

	try
	{
		po::variables_map vm;
		po::store(po::wcommand_line_parser(argc, argv).options(desc).run(), vm);

		if (vm.count("help"))
		{
			cout << desc << endl;
			return 0;
		}

		// Yes, the magic is putting the po::notify after "help" option check.
		po::notify(vm);

		if (!input.empty())
		{
			wcout << "virtualizing '" << input << "'" << endl;

			// make input absolute path
			input = sys_path_search(input);
			fs::path input_path(input);
			input = fs::absolute(input_path).wstring();

			wstring caps_str;
			if (!caps.empty())
			{
				caps_str = boost::algorithm::join(caps, L";");
			}
			else
			{
				// good defaults
				caps_str = L"clipboard";
			}

			wcout << "       input: " << input << endl;
			wcout << "capabilities: " << (caps_str.empty() ? L"none" : caps_str) << endl;

			// make app path absolute
			if (!app_path.empty())
			{
				app_path = sys_path_search(app_path);
				wcout << "         app: " << app_path << endl;
			}

			if (!fs::exists(input))
			{
				cout << endl << "error: input does not exist!" << endl;
				return 1;
			}

			fs::path output_path(output);
			if (output.empty())	// generate output file
			{
				output_path = fs::current_path() / input_path.filename();
			}
			output = fs::absolute(output_path).wstring();

			wcout << "      output: " << output << endl;
			wcout << "        args: " << (args.empty() ? L"<none>" : args) << endl;
			wcout << endl;

			if (input == output)
			{
				cout << endl << "error: input and output are the same!" << endl;
				return 1;
			}

			try
			{
				bool is_console = is_console_exe(input);
				wcout << "extracting " << (is_console ? L"terminal" : L"GUI") << " shim... ";

				{
					resources this_res;
					this_res.extract_binary_to_file(IDR_SHIM_EXE, output);
					cout << "ok." << endl;
				}

				resources shim_res(output);

				if (!no_mirror)
				{
					cout << "mirroring properties... ";
					resources input_res(input);
					cout << "ver... ";
					shim_res.replace_version_info(input_res);
					cout << "icon... ";
					shim_res.replace_icon(input_res);
					cout << "ok." << endl;
				}

				cout << "configuring startup sequence... ";
				vector<wstring> st;
				st.push_back(app_path.empty() ? input : app_path);
				st.push_back(args);
				st.push_back(caps_str);
				shim_res.replace_string_table(1, st);
				shim_res.commit_changes();
				cout << "ok." << endl;

				if (!is_console)
				{
					cout << "patching subsystem flag... ";
					patch_exe_subsystem(output, true);
					cout << "ok" << endl;
				}
			}
			catch (runtime_error& err)
			{
				cout << err.what() << endl;
			}
		}
		else
		{

		}

	}
	catch (std::exception& e)
	{
		cerr << "error: " << e.what() << "." << endl;
		cerr << "run with -h switch to list available options." << endl;
	}

	return 0;
}