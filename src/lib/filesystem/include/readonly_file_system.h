#pragma once

#include "svfs/definitions.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace MoonGlare {

class iReadOnlyFileSystem {
protected:
    virtual ~iReadOnlyFileSystem() = default;

public:
    // virtual bool OpenFile(ByteTable &FileData, StarVFS::FileID fid) = 0;
    virtual bool ReadFileByPath(const std::string &path, std::string &file_data) const = 0;

    // template <typename T> bool OpenXML(XMLFile &doc, const T &file) {
    //     doc.reset();
    //     StarVFS::ByteTable data;
    //     if (!OpenFile(data, file))
    //         return false;

    //     doc = std::make_unique<pugi::xml_document>();
    //     auto result = doc->load_string((char *)data.get());

    //     return static_cast<bool>(result);
    // }

    virtual bool EnumeratePath(const std::string_view &path, FileInfoTable &result_file_table) = 0;

    // virtual void FindFilesByExt(const char *ext, StarVFS::DynamicFIDTable &out) = 0;
    // virtual std::string GetFileName(StarVFS::FileID fid) const = 0;
    // virtual std::string GetFullFileName(StarVFS::FileID fid) const = 0;
    // virtual bool FileExists(const std::string &uri) const = 0;
};

} // namespace MoonGlare