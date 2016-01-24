#include PCH_HEADER
#include "mgdtSettings.h"
#include "MainForm.h"
#include "RemoteConsole.h"

using OrbitLogger::LogCollector;
using OrbitLogger::StdFileLoggerSink;

int main(int argc, char *argv[]) {

	OrbitLogger::ThreadInfo::SetName("MAIN", true);
	LogCollector::Start();
	LogCollector::OpenLogSink<StdFileLoggerSink>([](StdFileLoggerSink* sink) { sink->Open("logs/MGInsider.log"); });

	int r;
	//mgdtSettings::get().Load();
	QApplication a(argc, argv);
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
