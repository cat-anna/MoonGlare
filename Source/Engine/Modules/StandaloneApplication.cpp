#include <pch.h>

#include <boost/filesystem.hpp>

#include <nfMoonGlare.h>

#include <Foundation/Settings.h>

#include "../Application.h"

namespace MoonGlare {

class StandaloneApplication : public Application {
public:
    ~StandaloneApplication() { }

    std::string ApplicationPath() const override {
        return boost::filesystem::path(exeName).parent_path().string();
    }

    std::shared_ptr<Settings> GetUpperLayerSettings() override {
        auto stt = std::make_shared<Settings>();
        for (const auto &item : startupArguments)
            stt->AppendString(item);
        return stt;
    };

    StandaloneApplication(int argc, char** argv) {
        exeName = argv[0];
        if(argc > 1)
            startupArguments = std::vector<std::string>(argv + 1, argv + argc);
    }

    static int Entry(int argc, char** argv) {
        srand(time(NULL));

        using OrbitLogger::LogCollector;

        try {
            LogCollector::Start();
            do {
                auto app = std::make_unique<StandaloneApplication>(argc, argv);

                app->Execute();
                if (app->DoRestart()) {
                    AddLog(Debug, "Performing application restart");
                    continue;
                }

                app.reset();
            } while (false);
            OrbitLogger::LogCollector::Stop();
            return 0;
        }
        catch (const char * Msg) {
            AddLogf(Error, "FATAL ERROR! '%s'", Msg);
        }
        catch (const std::string & Msg) {
            AddLogf(Error, "FATAL ERROR! '%s'", Msg.c_str());
        }
        catch (std::exception &E) {
            AddLog(Error, "FATAL ERROR! '" << E.what() << "'");
        }
        catch (...) {
            AddLog(Error, "UNKNOWN FATAL ERROR!");
        }
        OrbitLogger::LogCollector::Stop();
        return 1;
    }

private:
    std::string exeName;
    std::vector<std::string> startupArguments;
};

} //namespace MoonGlare
                            
#ifndef _BUILDING_TOOLS_

int main(int argc, char** argv) {
    return MoonGlare::StandaloneApplication::Entry(argc, argv);
}

#endif // _BUILDING_TOOLS_
