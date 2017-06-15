/*
  * Generated by cppsrc.sh
  * On 2017-02-04 21:00:07,51
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef AssetFileSystem_H
#define AssetFileSystem_H

#include "../Engine/FileSystem.h"

namespace MoonGlare::Asset {

class FileSystem final {
public:
    FileSystem();
    ~FileSystem();

    void Initialize();
    void Finalize();

    bool OpenFile(const std::string& FileName, DataPath origin, StarVFS::ByteTable &FileData);
private: 
};

} //namespace MoonGlare::Asset 

#endif
