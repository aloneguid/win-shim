#include <iostream>
#include "boost/program_options.hpp"
#include "resources.h"

using namespace std;
namespace po = boost::program_options;

int main(int ac, char* av[])
{
	// tutorial: https://www.boost.org/doc/libs/1_77_0/doc/html/program_options/tutorial.html#id-1.3.32.4.3
	po::options_description desc("allowed options");
	desc.add_options()
		("target,t", po::value<string>(), "path to target executable")
		("output,o", po::value<string>()->default_value(""), "path to output executable")
		("help,h", "Produce help message");

	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, desc), vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		cout << desc << endl;
		return 0;
	}
	else if (vm.count("target"))
	{
		auto target = vm["target"].as<string>();
		auto output = vm["output"].as<string>();

		cout << "target: " << target << endl;
		cout << "output: " << output << endl;

		try
		{
			resources target_res(target);
			resources shim_res("shim.exe");
			cout << "updating command line..." << endl;
			vector<wstring> st;
			st.push_back(L"one");
			st.push_back(L"two");
			shim_res.replace_string_table(1, st);
			shim_res.commit_changes();
			cout << "shim generated." << endl;
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