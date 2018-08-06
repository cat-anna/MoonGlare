/*
  * Generated by cppsrc.sh
  * On 2016-10-13 18:53:51,09
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include PCH_HEADER

#include <fmt/format.h>

#include <icons.h>
#include <iFileProcessor.h>
#include <iFileIconProvider.h>
#include <iCustomEnum.h>
#include <Module.h>
#include <iIssueReporter.h>

#include <libs/LuaWrap/src/LuaDeleter.h>
#include <libs/LuaWrap/src/LuaException.h>

#include "../Windows/MainWindow.h"
#include "../FileSystem.h"

#pragma warning ( push, 0 )
#include <assimp/Importer.hpp>     
#include <assimp/scene.h>          
#include <assimp/postprocess.h>  
#pragma warning ( pop )

namespace MoonGlare {
namespace Editor {
namespace Processor {

struct AssimpProcessor
    : public QtShared::iFileProcessor {

    AssimpProcessor(QtShared::SharedModuleManager modmgr, QtShared::SharedSetEnum ModelEnum, std::string URI) : QtShared::iFileProcessor(std::move(URI)), ModelEnum(ModelEnum), moduleManager(modmgr) { }

    ProcessResult ProcessFile() override {
        ModelEnum->Add(m_URI);

        //std::string nodesubPath = m_URI + "@node://";
        std::string meshsubPath = m_URI + "@mesh://";

        try {
            auto fs = moduleManager->QuerryModule<FileSystem>();
            StarVFS::ByteTable bt;
            if (!fs->GetFileData(m_URI, bt)) {
                //todo: log sth
                throw std::runtime_error("Unable to read file: " + m_URI);
            }
            if (bt.byte_size() == 0) {
                //todo: log sth
            }

            Assimp::Importer importer;
            unsigned flags = aiProcess_JoinIdenticalVertices |/* aiProcess_PreTransformVertices | */
                aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_SortByPType;

            const aiScene* scene = importer.ReadFileFromMemory(bt.get(), bt.byte_size(), flags, strrchr(m_URI.c_str(), '.'));

            if (!scene) {
                //std::cout << "Unable to open file!\n";
                return ProcessResult::UnknownFailure;
            }

            //std::vector<std::pair<std::string, const aiNode*>> queue;
            //queue.emplace_back("", scene->mRootNode);

            //while (!queue.empty()) {
            //    auto item = queue.back();
            //    queue.pop_back();
            //    auto path = item.first;
            //    auto node = item.second;

            //    path += std::string("/") + node->mName.data;

            //    if (node->mNumMeshes > 0) {
            //        ModelEnum->Add(nodesubPath + path);// +fmt::format(" [{}]", node->mNumMeshes));
            //    }

            //    for (int i = node->mNumChildren - 1; i >= 0; --i) {
            //        queue.emplace_back(path, node->mChildren[i]);
            //    }
            //}

            for (unsigned i = 0; i < scene->mNumMeshes; ++i) {
                auto mesh = scene->mMeshes[i];
                ModelEnum->Add(meshsubPath + "*" + std::to_string(i));
                if (mesh->mName.length > 0) {
                    ModelEnum->Add(meshsubPath + mesh->mName.data);
                }
            }
        }
        catch (...) {
            return ProcessResult::UnknownFailure;

        }
        return ProcessResult::Success;
    }
private:
    QtShared::SharedSetEnum ModelEnum;
    QtShared::SharedModuleManager moduleManager;
};


//----------------------------------------------------------------------------------

struct AssimpProcessorModule
    : public QtShared::iModule
    , public QtShared::iFileProcessorInfo
    , public QtShared::iCustomEnumSupplier
    , public QtShared::iFileIconInfo {

    AssimpProcessorModule(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {}

    QtShared::SharedSetEnum ModelListEnum = std::make_shared<QtShared::SetEnum>("string:Mesh.Model");

    QtShared::SharedFileProcessor CreateFileProcessor(std::string URI) override {
        return std::make_shared<AssimpProcessor>(GetModuleManager(), ModelListEnum, std::move(URI));
    }

    std::vector<std::string> GetSupportedTypes() {
        return{ "3ds", "fbx", "blend" };
    }

    std::vector<std::shared_ptr<QtShared::iCustomEnum>> GetCustomEnums() const override  {
        return { ModelListEnum, };
    }

    std::vector<FileIconInfo> GetFileIconInfo() const override {
        return std::vector<FileIconInfo>{
            FileIconInfo{ "blend", ICON_16_3DMODEL_RESOURCE, },
            FileIconInfo{ "3ds", ICON_16_3DMODEL_RESOURCE, },
            FileIconInfo{ "fbx", ICON_16_3DMODEL_RESOURCE, },
        };
    }
};
QtShared::ModuleClassRgister::Register<AssimpProcessorModule> AssimpProcessorModuleReg("AssimpProcessorModule");

//----------------------------------------------------------------------------------

} //namespace Processor 
} //namespace Editor 
} //namespace MoonGlare 
