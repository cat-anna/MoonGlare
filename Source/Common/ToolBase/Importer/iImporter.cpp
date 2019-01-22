#include "iImporter.h"

#include <boost/algorithm/string/case_conv.hpp>

//#include "AssimpImporter.h"

namespace MoonGlare::Importer {

template<typename T>
struct ImporterFactoryImpl : public ImporterFactory {
    std::shared_ptr<iImporter> CreateImporter(SharedModuleManager smm, iImporter::ImporterCreateData) override {
        return std::make_shared<T>(std::move(smm), std::move(createData));
    }
};

//----------------------------------------------------------------------------------

void ImporterProvider::RegisterModule() {
    ModuleClassRegister::Register<ImporterProvider> ImporterProviderReg("ImporterProvider");
}                                           

ImporterProvider::ImporterProvider(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {}

bool ImporterProvider::PostInit() {
    //for (auto module : GetModuleManager()->QuerryInterfaces<QtShared::iFileIconInfo>()) {
    //    for (auto &item : module.m_Interface->GetFileIconInfo()) {
    //        m_FileIconMap[ToLower(item.m_Ext)] = item.m_Icon;
    //        AddLogf(Info, "Associated icon: %s->%s", item.m_Ext.c_str(), module.m_Module->GetModuleName().c_str());
    //    }
    //}

    //AddImporter({
    //    std::make_shared<ImporterFactoryImpl<AssimpImporter>>(), 
    //    "",//icon ICON_16_3DMODEL_RESOURCE
    //    { "blend", "fbx", },//exts
    //    });

    return true;
}

void ImporterProvider::AddImporter(ImporterInfo ii) {

}

const std::shared_ptr<ImporterProvider::ImporterInfo> ImporterProvider::GetImporter(const std::string &ext) const {
    return importerMap.at(boost::to_lower_copy(ext));
}

}
