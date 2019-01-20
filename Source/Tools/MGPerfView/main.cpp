
#include <qapplication.h>

#include "Windows/MainWindow.h"
#include <Foundation/OS/Path.h>
#include <ToolBase/AppConfig.h>
#include <ToolBase/Module.h>
#include <ToolBase/ModuleRegistration.h>
#include <ToolBase/QtLogSink.h>

#include <OrbitLogger/src/sink/FileSink.h>

using OrbitLogger::LogCollector;
using OrbitLogger::StdFileLoggerSink;

namespace {

MoonGlare::AppConfig::MapType GetAppConig() {
    return {
        {"AppName", "MGPerfView"},
        {"ConfigFile", MoonGlare::OS::GetSettingsDirectory() + "/MGPerfView.xml" },
    };
}

}

int main(int argc, char* argv[]) {
    OrbitLogger::ThreadInfo::SetName("MAIN", true);
    LogCollector::Start();
    //auto basep = boost::filesystem::path(argv[0]).parent_path();
    LogCollector::AddLogSink<StdFileLoggerSink>("logs/MGPerfView.log");

    using MoonGlare::PerfView::MainWindow;

    int r;
    QApplication a(argc, argv);
    qInstallMessageHandler(&MoonGlare::QtLogSink);

    MoonGlare::RegisterBaseModules();
    MoonGlare::ModuleClassRgister::Register<MainWindow> MainWindowReg("MainWindow");

    {
        auto modmgr = MoonGlare::ModuleManager::CreateModuleManager(GetAppConig());
        modmgr->Initialize();
        r = a.exec();
        modmgr->Finalize();
        modmgr.reset();
    }

    //MoonGlare::Editor::EditorSettings::getInstance().Save();
    LogCollector::Stop();
    return r;
}
