/*
  * Generated by cppsrc.sh
  * On 2016-07-22 11:09:18,36
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef FileSystem_H
#define FileSystem_H

#include <qobject.h>
#include "Notifications.h"

#include <StarVFS/core/nStarVFS.h>

#include <iFileProcessor.h>
#include <Module.h>

namespace StarVFS {
	class StarVFS;
	using SharedStarVFS = std::shared_ptr<StarVFS>;
}

namespace MoonGlare {
namespace Editor {

class AsyncFileProcessor;

class FileSystem 
	: public QObject
	, std::enable_shared_from_this<FileSystem> {
	Q_OBJECT;
public:
 	FileSystem(QtShared::SharedModuleManager modmgr);
 	virtual ~FileSystem();

	StarVFS::SharedStarVFS GetVFS() { return m_VFS; }

	bool GetFileData(const std::string &uri, StarVFS::ByteTable &data);
	bool SetFileData(const std::string &uri, StarVFS::ByteTable &data);
	bool CreateFile(const std::string &uri);
	bool CreateDirectory(const std::string &uri);

	void QueueFileProcessing(const std::string &URI);
public slots:
	void Reload();
signals:
	void Changed();
	void FileProcessorCreated(QtShared::SharedFileProcessor);
protected slots:
	void ProjectChanged(Module::SharedDataModule datamod);
private:
	std::mutex m_Mutex;
	StarVFS::SharedStarVFS m_VFS;
	std::string m_BasePath;
	Module::SharedDataModule m_Module;
	std::unordered_map<std::string, std::vector<std::weak_ptr<QtShared::iFileProcessorInfo>>> m_ExtFileProcessorList;
	std::unique_ptr<AsyncFileProcessor> m_AsyncFileProcessor;
    QtShared::SharedModuleManager moduleManager;

	bool TranslateURI(const std::string &uri, std::string &out);
};

using SharedFileSystem = std::shared_ptr<FileSystem>;

} //namespace Editor 
} //namespace MoonGlare 

#endif
