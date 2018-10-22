#include <string>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <map>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <ToolBase/Dump/AssimpDump.h>

int main(int argc, char ** argv) {
    namespace po = boost::program_options;

    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce this help message")
        ("input", po::value<std::string>(), "input file")
        ("output", po::value<std::string>(), "output")
        ;
    
    po::positional_options_description p;
    p.add("input", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
        options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help") || !vm.count("input")) {
        std::cout << desc << "\n";
        return 1;
    }

    std::string input = vm["input"].as<std::string>();
    std::string output = input + ".mdmp";

    if (vm.count("output")) {
        output = vm["output"].as<std::string>();
    }


	try {
        std::ofstream of(output);
        MoonGlare::Dump::AssimpDump(input, of);
	}
	catch (std::exception e) {
        std::cout << e.what();
	}

	return 0;
}
