#include PCH_HEADER
#include "mgdtSettings.h"
#include "MainForm.h"
#include "RemoteConsole.h"

int main(int argc, char *argv[]) {
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

	return r;
}
