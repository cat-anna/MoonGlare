#include <cstddef>
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <list>
#include <mutex>
#include <iostream>
#define BOOST_NUMERIC_CONVERSION_DETAIL_CONVERTER_FLC_12NOV2002_HPP
#define BOOST_NUMERIC_CONVERSION_CONVERTER_FLC_12NOV2002_HPP
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/process.hpp>

#include "arguments.h"
                                  
namespace MoonGlare::RDCC {

namespace po = boost::program_options;

//-------------------------------------------------------------------------------------------------

std::shared_ptr<RDCCConfig> ArgumentParser::Run(int argc, char **argv) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "Produce help message")
        //("version,v", "Print version information and exit")

        ("input,i", po::value<std::string>(), "Select input folder")
        ("output,o", po::value<std::string>(), "Select output file")
        ("filter,f", po::value<std::string>(), "Filter files from input folder")

        ("svfs", po::value<std::string>(), "Select svfs executable")

        //("no-validate", po::bool_switch(), "Do not validate")
        ("debug", po::bool_switch()->default_value(false), "Build debug mode module")
        ("verbose,v", po::bool_switch()->default_value(false), "Be verbose")
        ("dump-script", po::bool_switch()->default_value(false), "Print svfs script")

        //("remote", po::bool_switch(), "Start remote server")
        //("remote-port", po::value<int>()->default_value(0), "Set port for remote server")
        //("mount,m", po::value<std::vector<std::string>>(), "Mount container. format: FILE[:MOUNTPOINT]") //[:TYPE]
        //("script,s", po::value<std::vector<std::string>>(), "Execute scipt before entering CLI. It may be file or chunk of code")
        //("export,e", po::value<std::vector<std::string>>(), "Export vfs content. format: EXPORTER:OUTFILE[:BASEPATH[:PARAM=VALUE]]")
        //("list-exporters", po::bool_switch(), "List available exporters")
        //("no-bash-mode", po::bool_switch(), "disable Bash-like function call")
        ;

    try {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            exit(0);
        }

        if (vm.count("version")) {
            std::cout << "version\n";
            exit(0);
        }

        RDCCConfig config;

        config.exeName = argv[0];
        auto myName = boost::filesystem::path(argv[0]);
        config.svfsExecutable = myName.parent_path().string() + "/svfs" + myName.extension().string();
        config.luacExecutable = myName.parent_path().string() + "/luac" + myName.extension().string();

        auto getOption = [&vm](const char *name, auto &option, const char *missingError = nullptr) {
            if (!vm[name].empty())
                option = vm[name].as<std::remove_reference_t<decltype(option)>>();
            else
                if (missingError)
                    throw missingError;
        };

        getOption("input", config.inputDir, "Input must be specified");
        getOption("output", config.outputFile, "Output must be specified");

        getOption("filter", config.fileFilter);

        getOption("svfs", config.svfsExecutable);

        getOption("debug", config.debugBuild);
        getOption("verbose", config.verbose);
        getOption("dump-script", config.dumpScript);

        if(config.inputDir.back() != '\'' && config.inputDir.back() != '/')
            config.inputDir += "/";

        if (!boost::ends_with(config.outputFile, ".rdc"))
            config.outputFile += ".rdc";

        config.tmpPath = ".rdcc." + std::to_string(boost::this_process::get_id()) + "/"; 
        boost::filesystem::create_directories(config.tmpPath);

        return std::make_shared<RDCCConfig>(std::move(config));
    }
    catch (const char *e) {
        std::cout << e << "\n";
        exit(1);
    }
    catch (po::error &e) {
        std::cout << e.what() << "\n";
        exit(1);
    }
}
                                  
}
