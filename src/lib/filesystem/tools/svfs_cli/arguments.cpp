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

struct BaseStringSet {
    virtual ~BaseStringSet() {}
    virtual bool split(const std::string &value) = 0;

protected:
    static bool xsplit(const char *&input, char delim, std::string &first) {
        if (!input || !*input)
            return false;

        auto dd = strchr(input, delim);
        if (!dd) {
            first = input;
            input = nullptr;
        } else {
            first = std::string(input, dd - input);
            input = dd + 1;
        }

        return true;
    }

    static bool vecSplit(const char *input, std::vector<std::string> &vec, char delim) {
        while (input) {
            std::string s;
            if (!xsplit(input, delim, s))
                return false;
            vec.emplace_back(std::move(s));
        }
        return true;
    }

    static bool vec2map(const std::vector<std::string> &vec, std::map<std::string, std::string> &map, char delim) {
        for (auto &it : vec) {
            auto pos = it.find(delim);
            if (pos == std::string::npos) {
                map[it] = "1";
                continue;
            }
            auto name = it.substr(0, pos);
            auto value = it.substr(pos + 1, it.length() - pos - 1);
            map[name] = value;
        }
        return false;
    }

    template <class... ARGS> static bool xsplit(const char *input, char delim, std::string &first, ARGS &... args) {
        xsplit(input, ':', first);
        return xsplit(input, ':', args...);
    }
};

struct MountInfo : public BaseStringSet {
    std::string m_Path, m_MountPoint;
    virtual bool split(const std::string &value) override {
        xsplit(value.c_str(), ':', m_Path, m_MountPoint);
        if (m_MountPoint.empty())
            m_MountPoint = "/";
        if (m_MountPoint.front() != '/')
            m_MountPoint = std::string("/") + m_MountPoint;
        return true;
    }
    static MountInfo FromString(const std::string &value) {
        MountInfo r;
        r.split(value);
        return r;
    }
};

struct InjectInfo : public BaseStringSet {
    // std::string m_Path, m_MountPoint;
    virtual bool split(const std::string &value) override {
        return false; // xsplit(value.c_str(), m_Path, m_MountPoint);
    }
    static InjectInfo FromString(const std::string &value) {
        InjectInfo r;
        // r.split(value);
        return r;
    }
};

struct ExportInfo : public BaseStringSet {
    // std::string m_Path, m_MountPoint;
    std::string m_Exporter, m_OutFile, m_BasePath, m_Arguemnts;
    std::map<std::string, std::string> GetArgs() const {
        std::vector<std::string> vec;
        std::map<std::string, std::string> map;
        vecSplit(m_Arguemnts.c_str(), vec, ',');
        vec2map(vec, map, '=');
        return map;
    }
    virtual bool split(const std::string &value) override {
        xsplit(value.c_str(), ':', m_Exporter, m_OutFile, m_BasePath, m_Arguemnts);
        if (m_BasePath.empty())
            m_BasePath = "/";
        if (m_BasePath.front() != '/')
            m_BasePath = std::string("/") + m_BasePath;
        return true;
    }
    static ExportInfo FromString(const std::string &value) {
        ExportInfo r;
        r.split(value);
        return r;
    }
};

//-------------------------------------------------------------------------------------------------

namespace po = boost::program_options;

struct PrivData {
    po::options_description m_desc;
    po::variables_map m_vm;
    std::list<std::string> m_Lines;
    bool verbose = false;

    std::string GetInitScript() {
        std::stringstream ss;
        for (auto &it : m_Lines)
            ss << it << "\n";
        return ss.str();
    }

    void AddLine(std::string line) { m_Lines.emplace_back(std::move(line)); }
    template <class... ARGS> void AddLine(const char *fmt, ARGS... args) {
        AddLine(fmt::format(fmt, std::forward<ARGS>(args)...));
    }

    void AddPrint(const std::string &line) {
        if (verbose) {
            AddLine("print([===[{}]===])", line);
        }
    }

    template <class... ARGS> void AddPrint(const char *fmt, ARGS... args) {
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
#if 0
        if (m_vm["remote"].as<bool>()) {
            int port = m_vm["remote-port"].as<int>();
            std::string sport;
            if (port <= 0 || port >= 0xFFFF) {
                port = 0;
                sport = "<default>";
            } else {
                sport = std::to_string(port);
            }

            AddPrint("Starting remote server at port {}", sport.c_str());
            AddLine("RemoteServer = Register:CreateModule('RemoteModule')");
            if (port)
                AddLine("RemoteServer:SetAttribute('Port', '{}')", port);
            AddLine("RemoteServer:Enable()");
            AddLine("");
        }
#endif
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

        if (!m_vm["mount"].empty()) {
            AddPrint("Mounting containers...");
            for (auto &it : m_vm["mount"].as<std::vector<std::string>>()) {
                auto info = MountInfo::FromString(it);
                AddPrint("Mounting {} on {}", info.m_Path.c_str(), info.m_MountPoint.c_str());
                AddLine("mount([===[{}]===], [===[{}]===])", info.m_Path.c_str(), info.m_MountPoint.c_str());
            }
            AddLine("");
        }

        // for (auto &it : e.m_Mounts) {
        //	if (e.m_InitVerbose)
        //		printf("Mounting %s to %s\n", it.m_Path.c_str(), it.m_MountPoint.c_str());
        //	auto ret = svfs.OpenContainer(it.m_Path.c_str(), it.m_MountPoint.c_str(), 0);
        //	if (ret != ::StarVFS::VFSErrorCode::Success) {
        //		printf("Failed to mount container!\n");
        //		return false;
        //	}
        //}

        // inject
        //	if (!vm["inject"].empty())
        //		for (auto &it : vm["inject"].as<std::vector<std::string>>())
        //			m_Env.m_Injects.push_back(InitEnv::InjectInfo::FromString(it));

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

// export
#if 0
        if (!m_vm["export"].empty()) {
            AddPrint("Exporting vfs content...");
            AddLine("");
            int idx = 0;
            for (auto &it : m_vm["export"].as<std::vector<std::string>>()) {
                ++idx;
                auto info = ExportInfo::FromString(it);
                char valname[32];
                sprintf(valname, "Exporter_%02d", idx);

                AddPrint("Exporting %s to %s using %s with%s arguments %s", info.m_BasePath.c_str(),
                         info.m_OutFile.c_str(), info.m_Exporter.c_str(), info.m_Arguemnts.empty() ? "out" : "",
                         info.m_Arguemnts.c_str());

                AddLine("local %s = Register:CreateExporter([[%s]])", valname, info.m_Exporter.c_str());
                auto map = info.GetArgs();
                for (auto &it : map)
                    AddLine("%s:SetAttribute([[%s]], [[%s]])", valname, it.first.c_str(), it.second.c_str());

                AddLine("%s:DoExport([[%s]], [[%s]])", valname, info.m_BasePath.c_str(), info.m_OutFile.c_str());
                AddLine("");
            }
        }
#endif
        return true;
    }

    bool ProcessFinalSettings() {
        if (m_vm["list-container-classes"].as<bool>()) {
            AddPrint("List of known container classes:");
            AddLine(R"==(print(table.concat(table_from_sol(StarVfs:GetContainerClassList()), " ")))==");
        }
        if (m_vm["list-module-classes"].as<bool>()) {
            AddPrint("List of known module classes:");
            AddLine(R"==(print(table.concat(table_from_sol(StarVfs:GetModuleClassList()), " ")))==");
        }
        if (m_vm["list-exporter-classes"].as<bool>()) {
            AddPrint("List of known exporter classes:");
            AddLine(R"==(print(table.concat(table_from_sol(StarVfs:GetExporterClassList()), " ")))==");
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

            ("library,l", po::value<std::vector<std::string>>(), "Load lua library") //

            ("meta-module", po::bool_switch(), "Load MetaModule at startup") //

            ("script,s", po::value<std::vector<std::string>>(), "Load and execute lua chunk or file") //

            ("list-container-classes", po::bool_switch(), "List available exporters classes") //
            ("list-module-classes", po::bool_switch(), "List available container classes")    //
            ("list-exporter-classes", po::bool_switch(), "List available module classes")     //

            // ("remote", po::bool_switch(), "Start remote server")                                               //
            // ("remote-port", po::value<int>()->default_value(0), "Set port for remote server")                  //
            // ("mount,m", po::value<std::vector<std::string>>(), "Mount container. format: FILE[:MOUNTPOINT]")   //

            //[:TYPE]
            //			("inject,i",
            // po::value<std::vector<std::string>>(),
            //"Inject file into vfs. format:
            // SYSPATH:VPATH. Virtual path will be created
            // if not exits") //[:TYPE]

            // ("export,e", po::value<std::vector<std::string>>(),
            //  "Export vfs content. format: EXPORTER:OUTFILE[:BASEPATH[:PARAM=VALUE]]") //
            ;
    }

    bool Run(InitEnv &out, int argc, char **argv) {
        po::store(po::parse_command_line(argc, argv, m_desc), m_vm);
        po::notify(m_vm);

        if (m_vm.count("help") > 0) {
            std::cout << m_desc << "\n";
            // std::cout << "Arguments are processed in order:\n";
            // std::cout << "[other settings] -> lua libs -> mounts -> injects -> scripts -> exports -> enter cli\n\n";
            exit(0);
        }

        if (m_vm.count("version")) {
            std::cout << "version TODO\n";
            exit(0);
        }

        out.verbose = verbose = m_vm["verbose"].as<bool>();
        out.run_cli = !m_vm["no-cli"].as<bool>();
        out.bash_mode = !m_vm["no-bash-mode"].as<bool>();

        if (!GenInitScriptEnv())
            return false;
        if (!ProcessInitialSettings())
            return false;
        if (!ProcessMainPipeline())
            return false;
        if (!ProcessFinalSettings())
            return false;

        if (m_vm["print-init-script"].as<bool>()) {
            std::cout << GetInitScript();
            exit(0);
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
