#pragma once

#include <svfs/vfs_module.hpp>
#include <variant_argument_map.hpp>

namespace MoonGlare::StarVfs {

class DynamicFileContainer;

class MetaModule : public iVfsModule {
public:
    static constexpr const char *kClassName = "meta_module";

    MetaModule(iVfsModuleInterface *module_interface, const VariantArgumentMap &arguments);
    virtual ~MetaModule();

private:
    DynamicFileContainer *dynamic_container;

    std::string DumpFilePathHashMap() const;
    std::string DumpFileTree() const;
    std::string DumpResourceIdMap() const;
};

} // namespace MoonGlare::StarVfs
