#pragma once

#include <svfs/variant_argument_map.hpp>
#include <svfs/vfs_module.h>

namespace MoonGlare::StarVfs {

class DynamicFileContainer;

class MetaModule : public iVfsModule {
public:
    static constexpr const char *kClassName = "meta_module";

    MetaModule(iVfsModuleInterface *module_interface, const VariantArgumentMap &arguments);
    virtual ~MetaModule();

    // virtual bool Enable() override;
    // virtual bool Disable() override;

private:
    DynamicFileContainer *dynamic_container;
    // std::vector<Containers::SharedVirtualFileInterface> m_MetaFiles;

    std::string DumpFilePathHashMap() const;
    std::string DumpFileTree() const;
    std::string DumpFileContentHashMap() const;
};

} // namespace MoonGlare::StarVfs
