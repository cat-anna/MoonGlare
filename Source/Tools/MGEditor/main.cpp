#include PCH_HEADER
#include "EditorSettings.h"
#include "Windows/MainWindow.h"
#include <Foundation/OS/Path.h>
#include <ToolBase/Module.h>

#include <OrbitLogger/src/sink/FileSink.h>

using OrbitLogger::LogCollector;
using OrbitLogger::StdFileLoggerSink;

static void QtLogSink(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QByteArray localMsg = msg.toLocal8Bit();

    switch (type) {
    case QtDebugMsg:
        AddLogf(Debug, "%s(%s:%u, %s)", localMsg.constData(), context.file, context.line, context.function);
        break;
#if QT_VERSION >= 0x050500
    case QtInfoMsg:
        AddLogf(Info, "%s(%s:%u, %s)", localMsg.constData(), context.file, context.line, context.function);
        break;
#endif
    case QtWarningMsg:
        AddLogf(Warning, "%s(%s:%u, %s)", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        AddLogf(Error, "%s(%s:%u, %s)", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        AddLogf(Error, "%s(%s:%u, %s)", localMsg.constData(), context.file, context.line, context.function);
        abort();//??
    default:
        LogInvalidEnum(type);
        return;
    }
}

int main(int argc, char *argv[]) {

    //{
    //    (new MoonGlare::Editor::EditorSettings(MoonGlare::OS::GetSettingsDirectory()))->Load();
    //}

    OrbitLogger::ThreadInfo::SetName("MAIN", true);
    LogCollector::Start();
    //auto basep = boost::filesystem::path(argv[0]).parent_path();
    LogCollector::AddLogSink<StdFileLoggerSink>("logs/MGEditor.log");

    using MoonGlare::Editor::MainWindow;

    int r;
    QApplication a(argc, argv);
    qInstallMessageHandler(&QtLogSink);
    MoonGlare::ModuleClassRgister::Register<MainWindow> MainWindowReg("MainWindow");

    {
        auto modmgr = MoonGlare::ModuleManager::CreateModuleManager();
        modmgr->Initialize();
        modmgr->QuerryModule<MainWindow>()->show();
        r = a.exec();
        modmgr->Finalize();
        modmgr.reset();
    }

    MoonGlare::Editor::EditorSettings::getInstance().Save();
    LogCollector::Stop();
    return r;
}
