#include PCH_HEADER
#include "EditorSettings.h"
#include "Windows/MainWindow.h"

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

	(new MoonGlare::Editor::EditorSettings())->Load();

	OrbitLogger::ThreadInfo::SetName("MAIN", true);
	LogCollector::Start();
	LogCollector::OpenLogSink<StdFileLoggerSink>([](StdFileLoggerSink* sink) { sink->Open("logs/MGEditor.log"); });

	int r;
	QApplication a(argc, argv);
	qInstallMessageHandler(&QtLogSink);
	{
		MoonGlare::Editor::MainWindow w;
		w.show();
		r = a.exec();
	}

	MoonGlare::Editor::EditorSettings::getInstance().Save();
	LogCollector::Stop();
	return r;
}
