
#include PCH_HEADER

#include <filesystem>

#include "AssimpImporter.h"

#include <Foundation/OS/WaitForProcess.h>

#include <Foundation/Resources/Importer/AssimpAnimationImporter.h>
#include <Foundation/Resources/Importer/AssimpMeshImporter.h>

#include <Foundation/Resources/Blob/AnimationBlob.h>
#include <Foundation/Resources/Blob/MeshBlob.h>

#include "../FileSystem.h"
#include <ToolBase/Module.h>
#include <iEditor.h>
#include <iFileIconProvider.h>
#include <iFileProcessor.h>
#include <icons.h>

#pragma warning ( push, 0 )
#include <assimp/Importer.hpp>     
#include <assimp/postprocess.h>  
#include <assimp/scene.h>          
#pragma warning ( pop )

#include <DataModels/EditableEntity.h>

#include <gl/glew.h>

#include <Renderer/Configuration.Renderer.h>

#include <Renderer/Material.h>

#include <Math.x2c.h>

#include <Common.x2c.h>

#include <Material.x2c.h>

namespace MoonGlare::Importer {


using QtShared::DataModels::EditableEntity;

struct AssimpImporter 
    : public QtShared::iFileProcessor 
    , public QtShared::iEditor {

    AssimpImporter(SharedModuleManager modmgr, std::string URI) 
        : QtShared::iFileProcessor(std::move(URI)), moduleManager(modmgr) { 
    
    }
            
    bool OpenData(const std::string &URI)  override {
        m_URI = URI;

        auto dotpos = URI.rfind(".");
        if (dotpos == std::string::npos || dotpos < URI.rfind("/")) {
            outputDirectory = URI + ".imported";
        } 
        else {
            outputDirectory = URI.substr(0, dotpos); 
        }

        sourceDirectory = std::filesystem::path(URI).remove_filename().generic_string();

        ProcessFile();
        return true; 
    }

    void StoreResult() {
        auto fs = moduleManager->QuerryModule<Editor::FileSystem>();

        if (!fs->CreateDirectory(outputDirectory)) {
            __nop();
            //todo
            return;
        }

        auto storeFile = [&](const std::string &fname, StarVFS::ByteTable &bt) {
            auto outf = outputDirectory + "/" + fname;
            if (!fs->CreateFile(outf)) {
                __debugbreak();
                //ErrorMessage("Failed during creating epx file");
                AddLog(Hint, "Failed to create file: " << outf);
                //return;// false;
            }
            if (!fs->SetFileData(outf, bt)) {
                __debugbreak();
                AddLog(Hint, "Failed to write file: " << outf);
                //todo: log sth
                //return;// false;
            }
        };
        
        std::stringstream ss;
        ss << ".gitignore" << "\n";
        ss << "mdmp.txt" << "\n";

        for (auto &item : generatedFiles) {
            ss << item.first << "\n";
            storeFile(item.first, item.second);
        }

        StarVFS::ByteTable gi;
        gi.from_string(ss.str());
        storeFile(".gitignore", gi);


        auto qtpath = QApplication::applicationDirPath();
        std::string path = qtpath.toUtf8().constData();
        path += "/mdmp.exe";

        auto outF = fs->TranslateToSystem(outputDirectory) + "/mdmp.txt";
        auto inF = fs->TranslateToSystem(m_URI);

        OS::WaitForProcess({ path, "--input", inF, "--output", outF });
    }

    static StarVFS::ByteTable XmlToData(pugi::xml_document &doc) {
        std::stringstream ss;
        doc.save(ss);
        StarVFS::ByteTable bt;
        bt.from_string(ss.str());
        return std::move(bt);
    }

    void ImportBodyShapeComponent(const aiNode * node, EditableEntity * parent) {
        if (node->mNumMeshes != 1)
            return;

        MeshInfo mi = mesh[node->mMeshes[0]];
        auto shapeC = parent->AddComponent("BodyShape");
        auto shio = shapeC->GetValuesEditor();
        shio->Set("type", "4");
        shio->Set("size.x", std::to_string(std::max(mi.boxSize.x / 2, 0.01f)));
        shio->Set("size.y", std::to_string(std::max(mi.boxSize.y / 2, 0.01f)));
        shio->Set("size.z", std::to_string(std::max(mi.boxSize.z / 2, 0.01f)));
    }

    void ImportBodyComponent(const aiNode * node, EditableEntity * parent) {
        auto bodyC = parent->AddComponent("Body");
        auto bdio = bodyC->GetValuesEditor();
        bdio->Set("Kinematic", "1");
    }

    void ImportTransformComponent(const aiNode * node, EditableEntity * parent) {
        aiQuaternion q;
        aiVector3D pos;
        aiVector3D scale;
        node->mTransformation.Decompose(scale, q, pos);

        auto transform = parent->AddComponent("Transform");
        auto trio = transform->GetValuesEditor();
        trio->Set("Position.x", std::to_string(pos.x));
        trio->Set("Position.y", std::to_string(pos.y));
        trio->Set("Position.z", std::to_string(pos.z));


        trio->Set("Scale.x", std::to_string(scale.x));
        trio->Set("Scale.y", std::to_string(scale.y));
        trio->Set("Scale.z", std::to_string(scale.z));

        trio->Set("Rotation.x", std::to_string(q.x));
        trio->Set("Rotation.y", std::to_string(q.y));
        trio->Set("Rotation.z", std::to_string(q.z));
        trio->Set("Rotation.w", std::to_string(q.w));
    }

    void ImportMeshComponent(const aiNode * node, EditableEntity * parent) {
        if (node->mNumMeshes != 1)
            return;

        auto meshSrc = scene->mMeshes[node->mMeshes[0]];
        if(!meshSrc->HasBones()) {
            auto meshC = parent->AddComponent("Mesh");
            auto meio = meshC->GetValuesEditor();
            auto &meshinfo = mesh[node->mMeshes[0]];
            meio->Set("Mesh", meshinfo.uri);
            meio->Set("Material", material[meshinfo.material].uri);
            meio->Set("Visible", "1");
        } else {
            std::string localRels;
            for (size_t i = 0; i < meshSrc->mNumBones; ++i) {
                if (!localRels.empty())
                    localRels += ",";
                localRels += meshSrc->mBones[i]->mName.data;
            }

            auto skinC = parent->AddComponent("Skin");
            auto skio = skinC->GetValuesEditor();
            auto &meshinfo = mesh[node->mMeshes[0]];
            skio->Set("Mesh", meshinfo.uri);
            skio->Set("Material", material[meshinfo.material].uri);
            skio->Set("LocalRelations", localRels);
        }
    }

    void ImportLightComponent( EditableEntity * parent) {
        //auto it = sceneLights.find(parent->GetName());
        //if (it == sceneLights.end())
        //    return;
        //auto light = it->second;
        //auto lightC = parent->AddComponent(Core::SubSystemId::Light);
        //auto liio = lightC->GetValuesEditor();
    }

    void ImportEntities() {
        std::unique_ptr<QtShared::DataModels::EditableEntity> entity;

        entity = std::make_unique<EditableEntity>();

        std::vector<std::pair<EditableEntity*, const aiNode*>> queue;
        queue.emplace_back(entity.get(), scene->mRootNode);

        std::size_t cnt = 0;
        while (!queue.empty()) {
            ++cnt;
            auto item = queue.back();
            queue.pop_back();
            EditableEntity* parent = item.first;
            auto node = item.second;

            parent->SetName(node->mName.data);

            ImportTransformComponent(node, parent);
            ImportMeshComponent(node, parent);
            ImportLightComponent(parent);
            ImportBodyComponent(node, parent);
            ImportBodyShapeComponent(node, parent);

            for (int i = node->mNumChildren - 1; i >= 0; --i) {
                auto ch = node->mChildren[i];
                //if( ch->mNumMeshes == 1 )
                queue.emplace_back(parent->AddChild(), ch);
            }
        }

        entity->SetName(boost::filesystem::path(m_URI).filename().string());

        pugi::xml_document xdoc;
        if (!entity->Write(xdoc.append_child("Entity"))) {
            //AddLog(Hint, "Failed to write epx Entities: " << m_CurrentPatternFile);
            throw false;
        }
        generatedFiles["entity.epx"] = XmlToData(xdoc);
    }

    void ImportTextures() {
        if (!scene->HasTextures())
            return;

        textureNames.resize(scene->mNumTextures);
        for (unsigned i = 0; i < scene->mNumTextures; ++i) {
            auto tex = scene->mTextures[i];
            if (tex->mHeight == 0) {
                std::string path = fmt::format("texture_{:02}.{}", i, (const char*)tex->achFormatHint);
                textureNames[i] = path;
                generatedFiles[path].copy_from(tex->pcData, tex->mWidth);
            }
            else {
                __debugbreak();
                throw false;
                //out << "\tHeight: " << tex->mHeight << "\n";
            }
        }
    }

    void ImportMaterials() {
        if (!scene->HasMaterials())
            return;

        auto getFloaProp = [](aiMaterial *mat, const char *name, float default) -> float {
            try {
                unsigned cnt = 1;
                float v = 0;
                if (mat->Get(name, 0, 0, &v, &cnt) == aiReturn_SUCCESS) {
                    return v;
                }
            }
            catch (...) {}
            return default;
        };

        auto getVec4Prop = [](aiMaterial *mat, const char *name) -> math::fvec4 {
            try {
                unsigned cnt = 4;
                float v[4] = { 1,1,1,1 };
                if (mat->Get(name, 0, 0, v, &cnt) == aiReturn_SUCCESS) {
                    __nop();
                }
                return math::fvec4{ v[0], v[1], v[2], v[3] };
            }
            catch (...) { }
            return {};
        };

        auto getVec3Prop = [](aiMaterial *mat, const char *name, math::fvec3 def = { 1,1,1, }) -> math::fvec3 {
            try {
                unsigned cnt = 3;
                float v[3] = { 1,1,1 };
                if (mat->Get(name, 0, 0, v, &cnt) == aiReturn_SUCCESS) {
                    return math::fvec3{ v[0], v[1], v[2] };
                }
            }
            catch (...) {
            }
            return def;
        };

        auto getStringProp = [](aiMaterial *mat, const char *name) -> std::string {
            try {
                if (aiString v;  mat->Get(name, 0, 0, v) == aiReturn_SUCCESS) {
                    return v.data;
                }
                return "";
            }
            catch (...) {}
            return "";
        };

        material.resize(scene->mNumMaterials);
        for (unsigned i = 0; i < scene->mNumMaterials; ++i) {
            const auto &mat = scene->mMaterials[i];

            Renderer::MaterialTemplate matData;

            aiString path;
            aiTextureMapping mapping = (aiTextureMapping)0;
            unsigned int uvindex = 0;
            float blend = 0;
            aiTextureOp op = (aiTextureOp)0;
            aiTextureMapMode mapmode[2] = {};

            auto loadMap = [&](aiTextureType texType, Renderer::MaterialTemplate::Map &map) {
                if (mat->GetTextureCount(texType) > 0) {
                    if (mat->GetTexture(texType, 0, &path, &mapping, &uvindex, &blend, &op, mapmode) == AI_SUCCESS) {
                        map.enabled = true;
                        if (path.data[0] == '*') {
                            int index = strtol(&path.data[1], nullptr, 10);
                            map.texture = outputDirectory + "/" + textureNames[index];
                        }
                        else {
                            std::filesystem::path fpath = path.data;
                            std::string fn = fpath.filename().generic_string();
                            if (fn.empty()) {
                                fn = path.data;
                                while (fn.front() == '/')
                                    fn = fn.substr(1);
                            }
                            map.texture = sourceDirectory + fn;
                        }
                        switch (mapmode[0]) {
                        case aiTextureMapMode_Wrap:
                            map.edges = Renderer::Configuration::Texture::Edges::Repeat;
                                break;
                        case aiTextureMapMode_Clamp:
                        case aiTextureMapMode_Decal:
                        case aiTextureMapMode_Mirror:
                        default:
                            map.edges = Renderer::Configuration::Texture::Edges::Clamp;
                        }
                    }
                }
            };

            loadMap(aiTextureType_DIFFUSE, matData.diffuseMap);
            //loadMap(aiTextureType_SPECULAR, matData.specularMap);
            loadMap(aiTextureType_NORMALS, matData.normalMap);
            loadMap(aiTextureType_SHININESS, matData.shinessMap);
            
            matData.diffuseColor = getVec3Prop(mat, "$clr.diffuse");
            matData.specularColor = getVec3Prop(mat, "$clr.specular");
            matData.emissiveColor = getVec3Prop(mat, "$clr.emissive", { 0,0,0, });
            matData.shiness = getFloaProp(mat, "$mat.shininess", 32);

            pugi::xml_document xdoc;
            x2c::Renderer::MaterialTemplate_t_Write(xdoc.append_child("Material"), matData, nullptr);

            std::string fname;
            std::string matName = getStringProp(mat, "?mat.name");
            if(matName.empty())
                fname = fmt::format("material_{:02}.mat", i);
            else
                fname = fmt::format("material_{}.mat", matName);

            MaterialInfo mi;
            mi.name = fname;
            mi.uri = outputDirectory + "/" + fname;
            material[i] = mi;
            generatedFiles[fname] = XmlToData(xdoc);
        }
    }

    void ImportMeshes() {
        mesh.clear();
        if (!scene->HasMeshes())
            return;       
        mesh.resize(scene->mNumMeshes);
        for (unsigned i = 0; i < scene->mNumMeshes; ++i) {
            auto amesh = scene->mMeshes[i];
            
            auto &mi = mesh[i];
            auto &maxs = mi.boxSize;
            maxs = { 0,0,0 };

            aiVector3D center = { 0,0,0 };
            for (size_t vertid = 0; vertid < amesh->mNumVertices; vertid++) {
                center += amesh->mVertices[vertid];
            }
            center /= (float)amesh->mNumVertices;

            for (size_t vertid = 0; vertid < amesh->mNumVertices; vertid++) {
                aiVector3D vertex = amesh->mVertices[vertid];

                for (int v = 0; v < 3; ++v) {
                    maxs[v] = std::max(maxs[v], abs(center[v] - vertex[v]));
                }
            }

            mi.material = amesh->mMaterialIndex;
						
			std::string meshName;
			if (amesh->mName.length > 0)
				meshName = amesh->mName.C_Str();
			else
				meshName = fmt::format("{}", i);
			meshName += ".mesh";
            mi.uri = outputDirectory + "/" + meshName;

            {
                Resources::MeshSource ms;
                Resources::Importer::ImportAssimpMesh(scene, i, ms);
                Resources::Importer::MeshImport mimport;
                Resources::Importer::ImportMeshSource(ms, mimport);
                std::stringstream blob;
                Resources::Blob::WriteMeshBlob(blob, mimport.mesh);
                StarVFS::ByteTable bt;
                bt.from_string(blob.str());
                generatedFiles[meshName] = std::move(bt);
            }
        }
    }

    void ImportAnimations() {
        animationNames.resize(scene->mNumAnimations);
        for (unsigned i = 0; i < scene->mNumAnimations; ++i) {
            std::string animName = fmt::format("{}.anim", i);
            animationNames[i] = animName;

            auto fs = moduleManager->QuerryModule<Editor::FileSystem>();
            StarVFS::ByteTable bt;
            if (!fs->GetFileData(m_URI + ".xml", bt)) {
                //todo: log sth
                throw std::runtime_error("Unable to read file: " + m_URI);
            }

            pugi::xml_document xdoc;
            xdoc.load_string(bt.c_str());

            {
                Resources::Importer::AnimationImport ai;
                Resources::Importer::ImportAssimpAnimation(scene, i, xdoc.document_element(), ai);
                std::stringstream blob;
                Resources::Blob::WriteAnimationBlob(blob, ai.animation);
                StarVFS::ByteTable btgen;
                btgen.from_string(blob.str());
                generatedFiles[animName] = std::move(btgen);
            }
        }
    }

    void GatherLights() {
        sceneLights.clear();
        if (!scene->HasLights())
            return;
        for (unsigned i = 0; i < scene->mNumLights; ++i) {
            auto light = scene->mLights[i];
            sceneLights[light->mName.data] = light;
        }
    }

    ProcessResult ProcessFile() override {
        try {
            auto fs = moduleManager->QuerryModule<Editor::FileSystem>();
            StarVFS::ByteTable bt;
            if (!fs->GetFileData(m_URI, bt)) {
                //todo: log sth
                throw std::runtime_error("Unable to read file: " + m_URI);
            }
            if (bt.byte_size() == 0) {
                //todo: log sth
            }

            unsigned flags = aiProcess_JoinIdenticalVertices |/* aiProcess_PreTransformVertices | */
                aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_SortByPType;

            scene = importer.ReadFileFromMemory(bt.get(), bt.byte_size(), flags, strrchr(m_URI.c_str(), '.'));

            if (!scene) {
                return ProcessResult::UnknownFailure;
            }

            //output

            GatherLights();
            ImportMeshes();
            ImportAnimations();
            ImportTextures();
            ImportMaterials();
            ImportEntities();

            StoreResult();
        }
        catch (const std::exception &) {
            return ProcessResult::UnknownFailure;

        }
        return ProcessResult::Success;
    }
private:
    const aiScene* scene;
    Assimp::Importer importer;
    SharedModuleManager moduleManager;
    std::map<std::string, const aiLight*> sceneLights;
    std::set<std::string> skippedNodes;

    struct MeshInfo {
        math::fvec3 boxSize = { 1,1,1 };
        std::string uri;
        unsigned material;
    };
    std::vector<MeshInfo> mesh;

    struct MaterialInfo {
        std::string name;
        std::string uri;
    };
    std::vector<MaterialInfo> material;
    std::vector<std::string> animationNames;

    std::vector<std::string> textureNames;

    std::string outputDirectory;
    std::string sourceDirectory;
    std::map<std::string, StarVFS::ByteTable> generatedFiles;
};
//----------------------------------------------------------------------------------

class AssimpImporterInfo
	: public iModule	
	, public QtShared::iEditorInfo
    , public QtShared::iEditorFactory {
public:
	AssimpImporterInfo(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {}

	virtual std::vector<FileHandleMethodInfo> GetOpenFileMethods() const override {
		return std::vector<FileHandleMethodInfo> {
			FileHandleMethodInfo{ "blend", ICON_16_3DMODEL_RESOURCE, "Import to entity pattern", "open", },
			FileHandleMethodInfo{ "3ds", ICON_16_3DMODEL_RESOURCE, "Import to entity pattern", "open", },
			FileHandleMethodInfo{ "fbx", ICON_16_3DMODEL_RESOURCE, "Import to entity pattern", "open", },
		};
	}

    QtShared::SharedEditor GetEditor(const iEditorInfo::FileHandleMethodInfo &method, const EditorRequestOptions&options) const override {
        return std::make_shared<AssimpImporter>(GetModuleManager(), "");
    }
};

ModuleClassRegister::Register<AssimpImporterInfo> AssimpImporterInfoReg("AssimpImporter");

//----------------------------------------------------------------------------------

} //namespace MoonGlare::Importer 

