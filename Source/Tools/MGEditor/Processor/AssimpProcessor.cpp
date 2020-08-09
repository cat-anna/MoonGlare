/*
 * Generated by cppsrc.sh
 * On 2016-10-13 18:53:51,09
 * by Paweu
 */
/*--END OF HEADER BLOCK--*/
#include PCH_HEADER

#include <filesystem>
#include <fmt/format.h>

#include <ToolBase/Module.h>
#include <iCustomEnum.h>
#include <iFileIconProvider.h>
#include <iFileProcessor.h>
#include <iIssueReporter.h>
#include <icons.h>

#include <libs/LuaWrap/src/LuaDeleter.h>
#include <libs/LuaWrap/src/LuaException.h>

#include "../FileSystem.h"
#include "../Windows/MainWindow.h"

#pragma warning(push, 0)
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#pragma warning(pop)

namespace MoonGlare {
namespace Editor {
namespace Processor {

struct AssimpProcessor : public QtShared::iFileProcessor {

    AssimpProcessor(SharedModuleManager modmgr, QtShared::SharedSetEnum MeshEnum, QtShared::SharedSetEnum MaterialEnum,
                    QtShared::SharedSetEnum AnimationEnum, std::string URI)
        : QtShared::iFileProcessor(std::move(URI)), MeshEnum(MeshEnum), MaterialEnum(MaterialEnum),
          AnimationEnum(AnimationEnum), moduleManager(modmgr) {}

    ProcessResult ProcessFile() override {
        MeshEnum->Add(m_URI);

        // std::string nodesubPath = m_URI + "@node://";

        try {
            auto fs = moduleManager->QueryModule<FileSystem>();
            StarVFS::ByteTable bt;
            if (!fs->GetFileData(m_URI, bt)) {
                // todo: log sth
                throw std::runtime_error("Unable to read file: " + m_URI);
            }
            if (bt.byte_size() == 0) {
                // todo: log sth
            }

            Assimp::Importer importer;
            unsigned flags = aiProcess_JoinIdenticalVertices | /* aiProcess_PreTransformVertices | */
                             aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_SortByPType;

            const aiScene *scene =
                importer.ReadFileFromMemory(bt.get(), bt.byte_size(), flags, strrchr(m_URI.c_str(), '.'));

            if (!scene) {
                // std::cout << "Unable to open file!\n";
                return ProcessResult::UnknownFailure;
            }

            std::filesystem::path puri(m_URI);
            std::string uri = puri.replace_extension("").generic_string() + "/";

            for (unsigned i = 0; i < scene->mNumMeshes; ++i) {
                // auto mesh = scene->mMeshes[i];
                MeshEnum->Add(fmt::format("{}{}.mesh", uri, i));
            }

            for (unsigned i = 0; i < scene->mNumMaterials; ++i) {
                auto material = scene->mMaterials[i];
                // MaterialEnum->Add(fmt::format("{}material_{}.mat", uri, material.));

                try {
                    if (aiString v; material->Get("?mat.name", 0, 0, v) == aiReturn_SUCCESS) {
                        MaterialEnum->Add(fmt::format("{}material_{}.mat", uri, v.data));
                        // MaterialEnum->Add(materialsubPath + v.data);
                    }
                } catch (...) {
                }
            }

            for (unsigned i = 0; i < scene->mNumAnimations; ++i) {
                // auto anim = scene->mAnimations[i];
                AnimationEnum->Add(fmt::format("{}{}.anim", uri, i));
            }
        } catch (...) {
            return ProcessResult::UnknownFailure;
        }
        return ProcessResult::Success;
    }

private:
    QtShared::SharedSetEnum MeshEnum;
    QtShared::SharedSetEnum MaterialEnum;
    QtShared::SharedSetEnum AnimationEnum;
    SharedModuleManager moduleManager;
};

//----------------------------------------------------------------------------------

struct AssimpProcessorModule : public iModule,
                               public QtShared::iFileProcessorInfo,
                               public QtShared::iCustomEnumSupplier,
                               public QtShared::iFileIconInfo {

    AssimpProcessorModule(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {}

    QtShared::SharedSetEnum MeshListEnum;
    QtShared::SharedSetEnum MaterialListEnum;
    QtShared::SharedSetEnum AnimationListEnum;

    QtShared::SharedFileProcessor CreateFileProcessor(std::string URI) override {
        return std::make_shared<AssimpProcessor>(GetModuleManager(), MeshListEnum, MaterialListEnum, AnimationListEnum,
                                                 std::move(URI));
    }

    std::vector<std::string> GetSupportedTypes() { return {"3ds", "fbx", "blend"}; }

    std::vector<std::shared_ptr<QtShared::iCustomEnum>>
    GetCustomEnums(QtShared::CustomEnumProvider *provider) override {
        MeshListEnum = provider->CreateEnum("string:Mesh.Mesh");
        MaterialListEnum = provider->CreateEnum("string:Mesh.Material");
        AnimationListEnum = provider->CreateEnum("string:BoneAnimator.Animation");

        provider->SetAlias("string:Skin.Mesh", MaterialListEnum);
        provider->SetAlias("string:Skin.Material", MeshListEnum);
        return {};
    }

    std::vector<FileIconInfo> GetFileIconInfo() const override {
        return std::vector<FileIconInfo>{
            FileIconInfo{
                "blend",
                ICON_16_3DMODEL_RESOURCE,
            },
            FileIconInfo{
                "3ds",
                ICON_16_3DMODEL_RESOURCE,
            },
            FileIconInfo{
                "fbx",
                ICON_16_3DMODEL_RESOURCE,
            },
        };
    }
};
ModuleClassRegister::Register<AssimpProcessorModule> AssimpProcessorModuleReg("AssimpProcessorModule");

//----------------------------------------------------------------------------------

} // namespace Processor
} // namespace Editor
} // namespace MoonGlare
