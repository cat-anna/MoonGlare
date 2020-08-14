
#include "svfs_lua.h"
#include "luainterface.h"
#include <svfs_modules/assimp_container.hpp>
#include <svfs_modules/assimp_importer_module.hpp>

namespace MoonGlare::Tools::VfsCli {

SVfsLua::SVfsLua(SharedLua lua) : StarVirtualFileSystem(&class_register), lua(lua) {
    class_register.RegisterAll();
    class_register.RegisterModuleClass<SvfsModules::AssimpImporterModule>();
    class_register.RegisterContainerClass<SvfsModules::AssimpContainer>();
    Initialize();
}

SVfsLua::~SVfsLua() {}

//-------------------------------------------------------------------------------------------------

bool SVfsLua::Initialize() {
    auto &sol = lua->get_sol();

    auto classes = sol["classes"].get_or_create<sol::table>();
    classes.new_usertype<SVfsLua>("StarVFS",                                     //
                                  "MountContainer", &SVfsLua::LuaMountContainer, //
                                  "LoadModule", &SVfsLua::LuaLoadModule,         //
                                  "CreateExporter", &SVfsLua::LuaCreateExporter, //
                                  "EnumeratePath", &SVfsLua::LuaEnumeratePath,   //
                                  "ReadFileByPath", &SVfsLua::LuaReadFileByPath, //

                                  "GetModuleClassList", &SVfsLua::GetModuleClassList,       //
                                  "GetContainerClassList", &SVfsLua::GetContainerClassList, //
                                  "GetExporterClassList", &SVfsLua::GetExporterClassList    //
    );

    classes.new_usertype<FileInfo>("FileInfo",                              //
                                   "is_directory", &FileInfo::is_directory, //
                                   "file_name", &FileInfo::file_name        //
    );

    classes.new_usertype<StarVfs::iVfsExporter>("iVfsExporter",                                    //
                                                "StartExport", &StarVfs::iVfsExporter::StartExport //
    );

    sol["StarVfs"] = this;

    return true;
}

//-------------------------------------------------------------------------------------------------

} // namespace MoonGlare::Tools::VfsCli
