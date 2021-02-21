#pragma once

#include "svfs/hashes.hpp"
#include <fmt/format.h>
#include <string>
#include <string_view>
#include <vector>

namespace MoonGlare {

struct FileInfo {
    std::string_view file_name;
    bool is_directory;
    bool is_hidden;
    StarVfs::FilePathHash file_resource_id;
    StarVfs::FilePathHash file_path_hash;
    StarVfs::FilePathHash parent_path_hash;
};
using FileInfoTable = std::vector<FileInfo>;

class iReadOnlyFileSystem {
public:
    virtual ~iReadOnlyFileSystem() = default;

    virtual bool ReadFileByPath(std::string_view path, std::string &file_data) const = 0;
    virtual bool ReadFileByResourceId(FileResourceId resource, std::string &file_data) const = 0;

    virtual bool EnumeratePath(std::string_view path, FileInfoTable &result_file_table) const = 0;
    virtual bool FindFilesByExt(std::string_view ext, FileInfoTable &result_file_table) const = 0;

    // virtual void FindFilesByExt(const char *ext, StarVFS::DynamicFIDTable &out) = 0;
    // virtual std::string GetFileName(StarVFS::FileID fid) const = 0;
    // virtual std::string GetFullFileName(StarVFS::FileID fid) const = 0;
    // virtual bool FileExists(const std::string &full_path) const = 0;

    virtual FileResourceId GetResourceByPath(std::string_view path) const = 0;
    virtual std::string GetNameOfResource(FileResourceId resource, bool wants_full_path = true) const = 0;
};

#ifdef WANTS_GTEST_MOCKS
struct ReadOnlyFileSystemMock : public iReadOnlyFileSystem {
    MOCK_CONST_METHOD2(ReadFileByPath, bool(std::string_view, std::string &));
    MOCK_CONST_METHOD2(ReadFileByResourceId, bool(FileResourceId, std::string &));
    MOCK_CONST_METHOD2(EnumeratePath, bool(std::string_view, FileInfoTable &));
    MOCK_CONST_METHOD2(FindFilesByExt, bool(std::string_view, FileInfoTable &));
    MOCK_CONST_METHOD1(GetResourceByPath, FileResourceId(std::string_view));
    MOCK_CONST_METHOD2(GetNameOfResource, std::string(FileResourceId, bool));

    ReadOnlyFileSystemMock() {
        using namespace ::testing;
        EXPECT_CALL(*this, GetNameOfResource(_, _)).WillRepeatedly(::testing::Invoke([](auto id, auto) {
            return fmt::format("res@{:016x}", id);
        }));
    }
};
#endif

} // namespace MoonGlare
