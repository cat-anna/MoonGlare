#include <fstream>
#include "../StarVFSInternal.h"
#include "FSExporter.h"
#include <boost/filesystem.hpp>

namespace StarVFS {
namespace Exporters {

FSExporter::FSExporter(StarVFS *svfs) : iExporter(svfs) {
}

FSExporter::~FSExporter() {
}

//-----------------------------------------------------------------------------

ExportResult FSExporter::WriteLocalFile(const String &LocalFileName) {
    auto &inputfiles = GetFileList();
    for (size_t i = 1, j = inputfiles.size(); i < j; ++i) {
        auto &inf = inputfiles[i];

        if (inf.m_Flags.ValidFile()) {
            ByteTable ct;
            auto path = GetSVFS()->GetFullFilePath(inf.m_VFSFileID);
            auto fullpath = LocalFileName + path;
            auto dir = boost::filesystem::path(fullpath).parent_path().string();
            boost::filesystem::create_directories(dir);

            if (!GetSVFS()->GetFileData(inf.m_VFSFileID, ct)) {
                STARVFSErrorLog("Failed to read content of file %d (%s)", inf.m_VFSFileID, path.c_str());
            }
            else {
                std::ofstream of(fullpath, std::ios::out | std::ios::binary);
                of.write(ct.c_str(), ct.byte_size());
                of.close();
            }
        }
    }

    return ExportResult::Sucess;
}

//-----------------------------------------------------------------------------

} //namespace Exporters 
} //namespace StarVFS 
