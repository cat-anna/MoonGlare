#pragma once

#include "luainterface.h"
#include "svfs/star_virtual_file_system.hpp"
#include "svfs/vfs_module.hpp"
#include <iostream>
#include <memory>
#include <svfs/svfs_class_register.hpp>
#include <svfs/vfs_container.hpp>

namespace MoonGlare::Tools::VfsCli {

class SVfsLua : public StarVfs::StarVirtualFileSystem {
public:
    SVfsLua(SharedLua Lua);
    ~SVfsLua();

private:
    SharedLua lua;
    StarVfs::SvfsClassRegister class_register;

    bool Initialize();

    StarVfs::VariantArgumentMap SolTableToVarMap(sol::table &sol_table) {
        StarVfs::VariantArgumentMap args;
        for (auto &item : sol_table) {
            if (item.second.is<bool>()) {
                args.set(item.first.as<const char *>(), item.second.as<bool>());
            } else if (item.second.is<int>()) {
                args.set(item.first.as<const char *>(), static_cast<int64_t>(item.second.as<int>()));
            } else {
                args.set(item.first.as<const char *>(), std::string(item.second.as<const char *>()));
            }
        }
        return args;
    }

    StarVfs::iVfsContainer *LuaMountContainer(const std::string &container_class, sol::table args) {
        return this->MountContainer(container_class, SolTableToVarMap(args));
    }

    StarVfs::iVfsModule *LuaLoadModule(const std::string &module_class, sol::table args) {
        return this->LoadModule(module_class, SolTableToVarMap(args));
    }

    std::unique_ptr<StarVfs::iVfsExporter> LuaCreateExporter(const std::string &module_class, sol::table args) {
        return this->CreateExporter(module_class, SolTableToVarMap(args));
    }

    FileInfoTable LuaEnumeratePath(const std::string &path) {
        FileInfoTable result;
        if (!this->EnumeratePath(path, result)) {
            throw std::runtime_error("Failed to enumerate directory");
        }
        return result;
    }

    std::string LuaReadFileByPath(const std::string &path) {
        std::string result;
        if (!this->ReadFileByPath(path, result)) {
            throw std::runtime_error("Failed to read file");
        }
        return result;
    }

    auto GetModuleClassList() const { return class_register.GetRegisteredModuleClasses(); }
    auto GetContainerClassList() const { return class_register.GetRegisteredContainerClasses(); }
    auto GetExporterClassList() const { return class_register.GetRegisteredExporterClasses(); }

    //     int RawCreateContainer(lua_State *lua);

    // 	bool RawForcePath(const char* path) const {
    // 		return false;
    // 	}

    // 	::StarVFS::FileHandle RawOpenFile(const char* FileName, int ReadMode = (int)::StarVFS::RWMode::R, int
    // FileMode = (int)::StarVFS::OpenMode::OpenExisting) { 		if (!FileName) 			return
    // ::StarVFS::FileHandle(); return OpenFile(FileName, (::StarVFS::RWMode)ReadMode,
    // (::StarVFS::OpenMode)FileMode);
    // 	}

    // 	const char* RawGetFileName(int fid) const { return GetFileName((::StarVFS::FileID)fid); }
    //     int RawIsFileDirectory(int fid) const { return IsFileDirectory((::StarVFS::FileID)fid) ? 1 : 0; }
    //     int RawFindFile(const char *f) { return FindFile(f); }

    // 	void CoutDumpStructure() const { DumpStructure(std::cout); }
    // 	void CoutDumpFileTable() const { DumpFileTable(std::cout); }
    // 	void CoutDumpHashTable() const { DumpHashTable(std::cout); }
};

}; // namespace MoonGlare::Tools::VfsCli
