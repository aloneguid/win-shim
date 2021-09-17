#include <iostream>
#include "boost/program_options.hpp"
#include "resource.h"
#include "resources.h"
#include "util.h"
#include <filesystem>

using namespace std;
namespace po = boost::program_options;
namespace fs = std::filesystem;

int main(int ac, char* av[])
{
	wcout << L"Windows Shim Maker by @aloneguid (https://github.com/aloneguid/win-shim)" << endl << endl;

	// tutorial: https://www.boost.org/doc/libs/1_77_0/doc/html/program_options/tutorial.html#id-1.3.32.4.3
	po::options_description desc("allowed options");
	desc.add_options()
		//("target,t", po::value<string>(), "Path to target executable.")
		("output,o", po::value<string>()->default_value(""), "Path to output executable.")
		("cmd,c", po::value<string>()->default_value(""), "Command line to execute, use %s to capture the original. For instance 'vim.exe %s'. ")
		("mirror,m", po::value<string>()->default_value(""), "Mirror data from another executable module (.exe or .dll). Mirrors version information, file description.")
		("help,h", "Produce help message.");

	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, desc), vm);
	po::notify(vm);

	auto cmd = vm["cmd"].as<string>();

	if (vm.count("help"))
	{
		cout << desc << endl;
		return 0;
	}
	else if (!cmd.empty())
	{
		//auto target = vm["target"].as<string>();
		auto output = vm["output"].as<string>();
		auto mirror = str_to_wstr(vm["mirror"].as<string>());

		if (output.empty())
		{
			output = "shim.exe";
		}

		fs::path ofile(output);
		ofile = fs::absolute(ofile);

		//cout << "target: " << target << endl;
		cout << "   cmd: " << cmd << endl;
		cout << "output: " << output << endl;
		cout << endl;

		try
		{
			cout << "extracting shim to " << ofile << "... ";
			{
				resources this_res;
				this_res.extract_binary_to_file(IDR_SHIM_EXE, ofile);
				cout << "ok." << endl;
			}

			cout << "patching shim... " << endl;
			resources shim_res(ofile);

			if (!mirror.empty())
			{
				resources mirror_res(mirror);
				shim_res.replace_version_info(mirror_res);
				shim_res.replace_icon(mirror_res);
			}

			vector<wstring> st;
			st.push_back(str_to_wstr(cmd));
			shim_res.replace_string_table(1, st);
			shim_res.commit_changes();
			cout << "ok." << endl;
		}
		catch (runtime_error& err)
		{
			cout << err.what() << endl;
		}
	}
	else
	{
		cout << desc << endl;
		return 1;
	}

	return 0;
}