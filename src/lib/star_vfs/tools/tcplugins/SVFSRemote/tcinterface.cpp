
#include <cstring>

#include "SVFSRemote.h"
#include <boost/filesystem.hpp>

#define PLUGINAPI __stdcall 
#define PLUGINEXPORT extern "C" __declspec(dllexport) 

#ifdef X32
#pragma comment(linker, "/EXPORT:FsInit=_FsInit@16")
#pragma comment(linker, "/EXPORT:FsFindFirst=_FsFindFirst@8")
#pragma comment(linker, "/EXPORT:FsFindNext=_FsFindNext@8")
#pragma comment(linker, "/EXPORT:FsFindClose=_FsFindClose@4")
#pragma comment(linker, "/EXPORT:FsGetDefRootName=_FsGetDefRootName@8")
#pragma comment(linker, "/EXPORT:FsGetFile=_FsGetFile@16")
#endif

static SVFSRemote _Instance;

PLUGINEXPORT
int PLUGINAPI FsGetFile(char* RemoteName, char* LocalName, int CopyFlags, RemoteInfoStruct* ri) {
	if (!RemoteName || !LocalName)
		return FS_FILE_READERROR;

	if (CopyFlags & FS_COPYFLAGS_RESUME)
		return FS_FILE_NOTSUPPORTED;

	if (!(CopyFlags & FS_COPYFLAGS_OVERWRITE)) {
		if (boost::filesystem::exists(LocalName))
			return FS_FILE_EXISTS;
	}

	char xPath[MAX_PATH + 1] = {};
	strcpy(xPath, RemoteName);

	for (int i = 0; xPath[i]; ++i) {
		if (xPath[i] == '\\')
			xPath[i] = '/';
	}

	auto ret = _Instance.GetFile(xPath, LocalName);

	return ret ? FS_FILE_OK : FS_FILE_WRITEERROR;
}

PLUGINEXPORT
int PLUGINAPI FsInit(int PluginNr, tProgressProc pProgressProc, tLogProc pLogProc, tRequestProc pRequestProc) {
	_Instance.Init(PluginNr, pProgressProc, pLogProc, pRequestProc);
	return 0;
}

PLUGINEXPORT
HANDLE PLUGINAPI FsFindFirst(char* Path, WIN32_FIND_DATA *FindData) {
	if (!Path)
		return INVALID_HANDLE_VALUE;

	char xPath[MAX_PATH + 1] = {};
	strcpy(xPath, Path);

	for (int i = 0; xPath[i]; ++i) {
		if (xPath[i] == '\\')
			xPath[i] = '/';
	}

	TCSearchData *data = _Instance.FindFirst(xPath, FindData);
	if (!data)
		return INVALID_HANDLE_VALUE;

	return data->AsHandle();
}

PLUGINEXPORT
BOOL PLUGINAPI FsFindNext(HANDLE Hdl, WIN32_FIND_DATA *FindData) {
	auto data = TCSearchData::FromHandle(Hdl);
	if (!data) {
		STARVFSErrorLog("Invalid data ptr!");
		return false;
	}
	return _Instance.FindNext(data, FindData);
}

PLUGINEXPORT
int PLUGINAPI FsFindClose(HANDLE Hdl) {
	auto data = TCSearchData::FromHandle(Hdl);
	if (!data) {
		STARVFSErrorLog("Invalid data ptr!");
		return 0;
	}
	return _Instance.FindClose(data);
}

PLUGINEXPORT
void PLUGINAPI FsGetDefRootName(char* DefRootName, int maxlen) {
	strncpy(DefRootName, "StarVFSRemote", maxlen);
}
