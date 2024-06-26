#include "debug_dump.hpp"
#include "engine_runner/engine_runner.hpp"
#include "engine_runner/modules/remote_console.hpp"
#include <boost/program_options.hpp>
#include <filesystem>
#include <glfw_context/glfw_context.hpp>
#include <orbit_logger.h>
#include <orbit_logger/sink/file_sink.h>
#include <svfs/star_virtual_file_system.hpp>
#include <svfs/svfs_class_register.hpp>

namespace MoonGlare {

using namespace MoonGlare::Runner;

namespace {
constexpr auto kEngineCompilationDate = __DATE__ " at " __TIME__;
constexpr auto kApplicationName = "MoonGlare generic engine";
constexpr auto kVersionString = "0.4";
} // namespace

struct StandaloneApplicationConfiguration {
    EngineConfiguration engine;
    std::vector<std::string> modules;

    //TODO: move recon server to lua scripts in debug module
    bool remote_console_enabled = true;
};

inline void to_json(nlohmann::json &j, const StandaloneApplicationConfiguration &p) {
    j = {
        {"engine", p.engine},
        {"modules", p.modules},
    };
}

inline void from_json(const nlohmann::json &j, StandaloneApplicationConfiguration &p) {
    try {
        j.at("engine").get_to(p.engine);
    } catch (const std::exception &e) {
        AddLogf(Warning, "Failed to read engine config: %s", e.what());
    }
    j.at("modules").get_to(p.modules);
}

class StandaloneApplication : public EngineRunner {
public:
    ~StandaloneApplication() override = default;
    StandaloneApplication(int argc, char **argv)
        : executable_name(std::filesystem::absolute(std::filesystem::path(argv[0]))),
          executable_path(executable_name.parent_path()) {
        class_register.RegisterAll();
        LoadAppConfig();
        ParseArguments(argc, argv);

        if (ec.remote_console_enabled) {
            CreateModule<Modules::RemoteConsoleModule>();
        }
    }

    std::shared_ptr<iReadOnlyFileSystem> CreateFilesystem() override {
        filesystem =
            std::make_shared<StarVfs::StarVirtualFileSystem>(&class_register, IntSvfsHooks());
        return filesystem;
    }

    void LoadDataModules() override {
        for (auto &module_path : ec.modules) {
            VariantArgumentMap args;
            args.set("mount_point", std::string());
            if (std::filesystem::is_directory(module_path)) {
                args.set("host_path", module_path);
                filesystem->MountContainer("host_folder", args);
                continue;
            }
            const auto ext = std::filesystem::path(module_path).extension();
            if (ext == std::filesystem::path(".zip")) {
                args.set("zip_file_path", module_path);
                filesystem->MountContainer("zip", args);
                continue;
            }
            throw std::runtime_error("Unknown container type: " + module_path);
        }

        DebugDump("svfs.log", filesystem->DumpStructure());
    }

    std::shared_ptr<Renderer::iDeviceContext> CreateDeviceContext() override {
        return std::make_shared<GlfwContext::GlfwContext>();
    }

    EngineConfiguration LoadConfiguration() const { return ec.engine; }

    void ParseArguments(int argc, char **argv) {
        namespace po = boost::program_options;

        po::options_description desc("Allowed options");
        desc.add_options()                   //
            ("help", "produce help message") //
            ("module", po::value<std::vector<std::string>>(), "load module");

        po::positional_options_description p;
        p.add("module", -1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("module")) {
            for (auto &item : vm["module"].as<std::vector<std::string>>()) {
                ec.modules.emplace_back(item);
            }
        }
    }

    void LoadAppConfig() {
        auto config_file = executable_name;
        config_file.replace_extension(".json");
        try {
            std::ifstream config_stream(config_file, std::ios::in);
            nlohmann::json config_json;
            config_stream >> config_json;
            config_json.get_to(ec);
        } catch (const std::exception &e) {
            AddLogf(Warning, "Failed to read config: %s", e.what());

            std::ofstream config_stream(config_file, std::ios::out);
            nlohmann::json config_json = ec;
            config_stream << config_json.dump(4);
        }
    }

    std::string GetVersionString() const override { return kVersionString; }
    std::string GetCompilationDate() const override { return kEngineCompilationDate; }
    std::string GetApplicationName() const override { return kApplicationName; }

private:
    const std::filesystem::path executable_name;
    const std::filesystem::path executable_path;

    StandaloneApplicationConfiguration ec;
    std::shared_ptr<StarVfs::StarVirtualFileSystem> filesystem;
    StarVfs::SvfsClassRegister class_register;
    // std::string exeName;
};

} //namespace MoonGlare

using OrbitLogger::LogCollector;
using OrbitLogger::StdFileLoggerSink;

int Execute(int argc, char **argv) {
    try {
        do {
            MoonGlare::StandaloneApplication app(argc, argv);
            app.Execute();
            if (app.WantsSoftRestart()) {
                AddLog(Info, "Performing application soft restart");
                continue;
            }
        } while (false);
        return 0;
    } catch (const std::exception &e) {
        AddLog(Error, "FATAL ERROR! '" << e.what() << "'");
    } catch (...) {
        AddLog(Error, "UNKNOWN FATAL ERROR!");
    }
    return 1;
}

int main(int argc, char **argv) {

    OrbitLogger::ThreadInfo::SetName("MAIN", true);
    LogCollector::Start();
    LogCollector::AddLogSink<StdFileLoggerSink>("logs/engine.log");
    LogCollector::AddLogSink<StdFileLoggerSink>("logs/engine.session.log", false);

    LogCollector::SetCaptureStdOut(OrbitLogger::LogChannels::StdOut);
    LogCollector::SetCaptureStdErr(OrbitLogger::LogChannels::StdErr);
    LogCollector::SetChannelName(OrbitLogger::LogChannels::StdOut, "SOUT");
    LogCollector::SetChannelName(OrbitLogger::LogChannels::StdErr, "SERR");
    LogCollector::SetChannelName(OrbitLogger::LogChannels::Performance, "PERF");

    int r = Execute(argc, argv);

    OrbitLogger::LogCollector::Stop();

    return r;
}
