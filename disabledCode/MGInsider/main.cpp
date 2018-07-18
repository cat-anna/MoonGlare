#include PCH_HEADER
#include "mgdtSettings.h"
#include "MainForm.h"
#include "RemoteConsole.h"

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
	OrbitLogger::ThreadInfo::SetName("MAIN", true);
	LogCollector::Start();
	LogCollector::OpenLogSink<StdFileLoggerSink>([](StdFileLoggerSink* sink) { sink->Open("logs/MGInsider.log"); });

	(new mgdtSettings())->Load();

	int r;
	//mgdtSettings::get().Load();
	QApplication a(argc, argv);
	qInstallMessageHandler(&QtLogSink);
	{
		MainForm w;
		w.show();
		r = a.exec();
	}
	GetRemoteConsole().Delete();
	mgdtSettings::get().Save();

	LogCollector::Stop();
	return r;
}
