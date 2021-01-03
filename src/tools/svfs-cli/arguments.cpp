#include "arguments.h"
#include <boost/program_options.hpp>
#include <cstddef>
#include <filesystem>
#include <fmt/format.h>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace MoonGlare::Tools::VfsCli {

//-------------------------------------------------------------------------------------------------

namespace po = boost::program_options;

struct PrivData {
    po::options_description m_desc;
    po::variables_map m_vm;
    std::list<std::string> m_Lines;
    bool verbose = false;
    bool imply_no_cli = false;

    std::string GetInitScript() {
        std::stringstream ss;
        for (auto &it : m_Lines)
            ss << it << "\n";
        return ss.str();
    }

    void AddLine(std::string line) { m_Lines.emplace_back(std::move(line)); }
    template <class... ARGS>
    void AddLine(const char *fmt, ARGS... args) {
        AddLine(fmt::format(fmt, std::forward<ARGS>(args)...));
    }

    void AddPrint(const std::string &line) {
        if (verbose) {
            AddLine("print([===[{}]===])", line);
        }
    }

    template <class... ARGS>
    void AddPrint(const char *fmt, ARGS... args) {
        if (verbose) {
            AddLine(fmt::format("print([===[{}]===])", fmt::format(fmt, std::forward<ARGS>(args)...)));
        }
    }

    bool GenInitScriptEnv() {
        AddLine("--StarVFS cli init script");
        AddLine("--Generated automatically from commandline");
        AddLine("");
        return true;
    }

    bool ProcessInitialSettings() {
        if (m_vm["meta-module"].as<bool>()) {
            AddPrint("Loading MetaModule");
            AddLine(R"==(load_module("meta_module"))==");
        }
        return true;
    }

    bool ProcessMainPipeline() {
        if (!m_vm["library"].empty()) {
            AddPrint("Loading Lua libs...");
            for (auto &it : m_vm["library"].as<std::vector<std::string>>()) {
                AddPrint("Loading library: {}", it.c_str());
                AddLine("require([===[{}]===])", it.c_str());
            }
            AddLine("");
        }

        if (!m_vm["script"].empty()) {
            AddPrint("Executing scripts...");
            for (auto &it : m_vm["script"].as<std::vector<std::string>>()) {
                bool isfile = std::filesystem::is_regular_file(it);
                if (isfile)
                    AddPrint("Executing file: {}", it);
                else
                    AddPrint("Executing chunk: {}", it);
                if (isfile) {
                    AddLine("dofile([===[{}]===])", it);
                } else {
                    AddLine("{}", it);
                }
            }
            AddLine("");
        }

        if (!m_vm["action"].empty()) {
            AddPrint("Executing actions...");
            for (auto &it : m_vm["action"].as<std::vector<std::string>>()) {
                AddLine("actions.execute_action([===[{}]===])", it);
            }
            AddLine("");
            imply_no_cli = true;
        }

        return true;
    }

    bool ProcessFinalSettings() {
        if (m_vm["list-container-classes"].as<bool>()) {
            AddPrint("List of known container classes:");
            AddLine(R"==(print(table.concat(table_from_sol(StarVfs:GetContainerClassList()), " ")))==");
            imply_no_cli = true;
        }
        if (m_vm["list-module-classes"].as<bool>()) {
            AddPrint("List of known module classes:");
            AddLine(R"==(print(table.concat(table_from_sol(StarVfs:GetModuleClassList()), " ")))==");
            imply_no_cli = true;
        }
        if (m_vm["list-exporter-classes"].as<bool>()) {
            AddPrint("List of known exporter classes:");
            AddLine(R"==(print(table.concat(table_from_sol(StarVfs:GetExporterClassList()), " ")))==");
            imply_no_cli = true;
        }
        if (m_vm["list-actions"].as<bool>()) {
            AddPrint("List of predefined actions");
            AddLine(R"==(actions.list_actions())==");
            imply_no_cli = true;
        }
        return true;
    }

    PrivData() : m_desc("Allowed options") {
        m_desc.add_options()                                                                                   //
            ("help", "produce help message")                                                                   //
            ("print-init-script", po::bool_switch(), "Print init script generated from command line and exit") //
            ("no-bash-mode", po::bool_switch(), "disable Bash-like function call")                             //
            ("version", "Print version information and exit")                                                  //
            ("verbose,v", po::bool_switch(), "Increase verbosity")                                             //
            ("no-cli", po::bool_switch(), "Do not enter CLI")                                                  //
            ("cli", po::bool_switch(), "Force entering CLI")                                                   //
            ("log", po::value<std::string>(), "redirect log to file")                                          //

            ("library,l", po::value<std::vector<std::string>>(), "Load lua library") //

            ("meta-module", po::bool_switch(), "Load MetaModule at startup") //

            ("script,s", po::value<std::vector<std::string>>(), "Load and execute lua chunk or file") //

            ("list-container-classes", po::bool_switch(), "List available exporters classes, implies --no-cli") //
            ("list-module-classes", po::bool_switch(), "List available container classes, implies --no-cli")    //
            ("list-exporter-classes", po::bool_switch(), "List available module classes, implies --no-cli")     //

            ("action", po::value<std::vector<std::string>>(), "Execute one of predefined actions, implies --no-cli") //
            ("list-actions", po::bool_switch(), "List available predefined actions, implies --no-cli")               //

            ;
    }

    bool Run(InitEnv &out, int argc, char **argv) {
        // for (int i = 0; i < argc; ++i)
        // std::cout << argv[i] << "\n";

        po::store(po::parse_command_line(argc, argv, m_desc), m_vm);
        po::notify(m_vm);

        if (m_vm.count("help") > 0) {
            std::cout << m_desc << "\n";
            exit(0);
        }

        if (m_vm.count("version")) {
            std::cout << "version TODO\n";
            exit(0);
        }

        out.verbose = verbose = m_vm["verbose"].as<bool>();
        out.bash_mode = !m_vm["no-bash-mode"].as<bool>();

        if (!GenInitScriptEnv())
            return false;
        if (!ProcessInitialSettings())
            return false;
        if (!ProcessMainPipeline())
            return false;
        if (!ProcessFinalSettings())
            return false;

        out.run_cli = !m_vm["no-cli"].as<bool>();
        if (imply_no_cli) {
            out.run_cli = false;
        }
        out.run_cli = out.run_cli || m_vm["cli"].as<bool>();

        if (m_vm["print-init-script"].as<bool>()) {
            std::cout << GetInitScript();
            exit(0);
        }

        if (m_vm.count("log")) {
            out.log_file = m_vm["log"].as<std::string>();
        }
        return true;
    }
};

//-------------------------------------------------------------------------------------------------

bool Parser::Run(InitEnv &out, int argc, char **argv) {
    auto data = std::make_unique<PrivData>();
    if (!data->Run(out, argc, argv))
        return false;

    std::stringstream ss;
    for (auto &it : data->m_Lines)
        ss << it << "\n";

    out.startup_script = ss.str();

    return true;
}

} // namespace MoonGlare::Tools::VfsCli
