/*
  * Generated by cppsrc.sh
  * On 2017-02-04 21:00:07,51
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/
#include "FileSystem.h"

namespace MoonGlare::Asset {

FileSystem::FileSystem() {
	//::MoonGlare::FileSystem::MoonGlareFileSystem::Instance();
}

FileSystem::~FileSystem() {
}

void FileSystem::Initialize() {
}

void FileSystem::Finalize() {
}

bool FileSystem::OpenFile(const std::string & FileName, DataPath origin, StarVFS::ByteTable & FileData) {
	return MoonGlare::FileSystem::MoonGlareFileSystem::Instance()->OpenFile(FileName, origin, FileData);
}

} //namespace MoonGlare::Asset
