#include "assimp_container.hpp"
#include <algorithm>
#include <dump/assimp_dump.h>
#include <editable_entity.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <json_helpers.hpp>
#include <libzippp.h>
#include <nlohmann/json.hpp>
#include <orbit_logger.h>
#include <resources/blob/animation_blob.hpp>
#include <resources/blob/mesh_blob.hpp>
#include <resources/importer/AssimpAnimationImporter.hpp>
#include <resources/importer/AssimpMeshImporter.hpp>
#include <stdexcept>
#include <svfs/file_table_interface.hpp>
#include <svfs/path_utils.hpp>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std::string_view_literals;

#pragma warning(push, 0)
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#pragma warning(pop)

namespace MoonGlare::Tools::SvfsModules {

struct AssimpLoader {
    std::string mount_point;
    StarVfs::FileResourceId source_resource_id;
    StarVfs::Hasher::SeedType my_seed;
    SharedModuleManager module_manger;
    StarVfs::FilePathHash parent_id;

    const aiScene *scene;
    Assimp::Importer importer;
    std::unordered_map<StarVfs::FilePathHash, std::unique_ptr<AssimpContainer::FileInfo>> loaded_files;
    std::unordered_set<std::string> generated_files;

    struct MeshInfo {
        glm::fvec3 box_size = {};
        // std::string uri;
        unsigned material_index;
    };
    std::vector<MeshInfo> known_meshes;
    std::vector<std::string> animation_names;

    StarVfs::HostFileSvfsManifest *current_manifest = nullptr;

    AssimpLoader(SharedModuleManager module_manger, StarVfs::FileResourceId source_resource_id, std::string mount_point)
        : module_manger(std::move(module_manger)), mount_point(std::move(mount_point)),
          source_resource_id(source_resource_id) {
        my_seed = source_resource_id;
        parent_id = StarVfs::Hasher::Hash(this->mount_point);
    }

    void AddFile(std::string file_data, std::string file_name) {
        auto fi = std::make_unique<AssimpContainer::FileInfo>();

        auto resource_id = StarVfs::Hasher::Hash(file_name, my_seed);
        StarVfs::HostFileSvfsManifestSubContent sub_entry;
        sub_entry.resource_id = resource_id;
        sub_entry.file_name = file_name;
        current_manifest->sub_contents.push_back(sub_entry);

        fi->resource_id = resource_id;
        fi->path_id = StarVfs::Hasher::Hash(StarVfs::JoinPath(mount_point, file_name));
        fi->container_id = fi->path_id;
        fi->parent_id = parent_id;

        fi->file_name = std::move(file_name);
        fi->file_data = std::move(file_data);
        loaded_files[fi->container_id] = std::move(fi);
        generated_files.insert(file_name);
    }

    void DumpContentInfo() {
        std::stringstream buffer;
        MoonGlare::Dump::AssimpDump(scene, buffer);
        AddFile(buffer.str(), ".mdmp");
    }

    void ImportMeshes() {
        known_meshes.clear();
        known_meshes.resize(scene->mNumMeshes);
        for (unsigned mesh_index = 0; mesh_index < scene->mNumMeshes; ++mesh_index) {
            const auto *assimp_mesh_ptr = scene->mMeshes[mesh_index];
            auto &mesh_info = known_meshes[mesh_index];

            aiVector3D center = {0, 0, 0};
            for (size_t vertid = 0; vertid < assimp_mesh_ptr->mNumVertices; vertid++) {
                center += assimp_mesh_ptr->mVertices[vertid];
            }
            center /= static_cast<float>(assimp_mesh_ptr->mNumVertices);

            for (size_t vertid = 0; vertid < assimp_mesh_ptr->mNumVertices; vertid++) {
                aiVector3D vertex = assimp_mesh_ptr->mVertices[vertid];

                for (int v = 0; v < 3; ++v) {
                    mesh_info.box_size[v] = std::max(mesh_info.box_size[v], std::abs(center[v] - vertex[v]));
                }
            }

            mesh_info.material_index = assimp_mesh_ptr->mMaterialIndex;

            std::string mesh_name;
            if (assimp_mesh_ptr->mName.length > 0)
                mesh_name = assimp_mesh_ptr->mName.C_Str();
            else
                mesh_name = fmt::format("{:03}", mesh_index);
            mesh_name += ".mesh";

            // mi.uri = outputDirectory + "/" + meshName;

            Resources::MeshSource mesh_source;
            Resources::Importer::ImportAssimpMesh(scene, mesh_index, mesh_source);
            Resources::Importer::MeshImport mesh_import;
            Resources::Importer::ImportMeshSource(mesh_source, mesh_import);
            std::stringstream blob;
            Resources::Blob::WriteMeshBlob(blob, mesh_import.mesh);
            AddFile(blob.str(), mesh_name);
        }
    }

    void ImportTextures() {
        std::unordered_map<XXH64_hash_t, unsigned> textureHashes;

        // textureNames.resize(scene->mNumTextures);

        for (unsigned texture_index = 0; texture_index < scene->mNumTextures; ++texture_index) {
            const auto *assimp_texture = scene->mTextures[texture_index];
            if (assimp_texture->mHeight == 0) {
                auto texture_content_hash = XXH64(assimp_texture->pcData, assimp_texture->mWidth, 'text');
                if (auto it = textureHashes.find(texture_content_hash); it != textureHashes.end()) {
                    // auto mappedTo = it->second;
                    // textureNames[i] = textureNames[mappedTo];
                } else {
                    textureHashes[texture_content_hash] = texture_index;
                    std::string file_name =
                        fmt::format("{:03}.{}", texture_index, (const char *)assimp_texture->achFormatHint);
                    AddFile(std::string(reinterpret_cast<const char *>(assimp_texture->pcData), assimp_texture->mWidth),
                            file_name);
                    // textureNames[i] = path;
                    // generatedFiles[path].copy_from(tex->pcData, tex->mWidth);
                }
            } else {
                __debugbreak();
                throw false;
                // out << "\tHeight: " << tex->mHeight << "\n";
            }
        }
    }

    void ImportAnimations() {
#if 0
        animation_names.resize(scene->mNumAnimations);

        for (unsigned i = 0; i < scene->mNumAnimations; ++i) {
            std::string anim_name = fmt::format("{:02}.anim", i);
            animation_names[i] = anim_name;

            auto fs = moduleManager->QueryModule<Editor::FileSystem>();
            StarVFS::ByteTable bt;
            if (!fs->GetFileData(m_URI + ".xml", bt)) {
                // todo: log sth
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
#endif
    }

    void ImportTransformComponent(const aiNode *node, std::shared_ptr<EditableEntity> parent) {
        aiQuaternion q;
        aiVector3D pos;
        aiVector3D scale;
        node->mTransformation.Decompose(scale, q, pos);

        auto transform = parent->AddComponent("Transform");
        auto component_data = transform->GetComponentData();
        using VariantType = VariantArgumentMap::VariantType;
        component_data->SetValue("position", VariantType(math::fvec3(pos.x, pos.y, pos.z)));
        component_data->SetValue("scale", VariantType(math::fvec3(scale.x, scale.y, scale.z)));
        component_data->SetValue("quaternion", VariantType(math::Quaternion(q.x, q.y, q.z, q.w)));
    }

    void ImportEntities() {
        for (int i = scene->mRootNode->mNumChildren - 1; i >= 0; --i) {
            auto root_entity = std::make_shared<EditableEntity>(module_manger);
            auto root_node = scene->mRootNode->mChildren[i];

            std::vector<std::pair<std::shared_ptr<EditableEntity>, const aiNode *>> queue;
            queue.emplace_back(root_entity, root_node);

            std::size_t cnt = 0;
            while (!queue.empty()) {
                ++cnt;
                auto item = queue.back();
                queue.pop_back();
                std::shared_ptr<EditableEntity> parent = item.first;
                auto node = item.second;
                parent->SetName(node->mName.data);

                ImportTransformComponent(node, parent);
                // ImportMeshComponent(node, parent);
                // ImportLightComponent(parent);
                // ImportBodyComponent(node, parent);
                // ImportBodyShapeComponent(node, parent);

                for (int i = node->mNumChildren - 1; i >= 0; --i) {
                    auto ch = node->mChildren[i];
                    // if( ch->mNumMeshes == 1 )
                    queue.emplace_back(parent->AddChild(), ch);
                }
            }

            std::string name = root_node->mName.data;
            std::string fname = name + ".entity";
            for (int idx = 0; name.empty() || generated_files.find(fname) != generated_files.end(); ++i) {
                if (root_node->mName.length > 0)
                    name = root_node->mName.data + std::string(".") + std::to_string(i);
                else
                    name = std::to_string(i);
                fname = name + ".entity";
            }
            root_entity->SetName(name);
            AddFile(root_entity->SerializeToJson(), fname);
        }
    }

    bool Import(const std::string_view &file_ext, const std::string_view file_data,
                StarVfs::HostFileSvfsManifest &file_manifest) {
        current_manifest = &file_manifest;
        unsigned flags = aiProcess_JoinIdenticalVertices | /* aiProcess_PreTransformVertices | */
                         aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_SortByPType;

        scene = importer.ReadFileFromMemory(file_data.data(), file_data.size(), flags, std::string(file_ext).c_str());

        if (!scene) {
            return false;
        }

        DumpContentInfo();
        ImportMeshes();
        ImportTextures();
        ImportAnimations();

        ImportEntities();

        current_manifest = nullptr;
        return true;
    }
};

//-------------------------------------------------------------------------------------------------

using ContainerFileEntry = StarVfs::iFileTableInterface::ContainerFileEntry;

//-------------------------------------------------------------------------------------------------

AssimpContainer::AssimpContainer(StarVfs::iFileTableInterface *fti, const VariantArgumentMap &arguments)
    : iVfsContainer(fti) {
    mount_point = StarVfs::OptimizeMountPointPath(arguments.get<std::string>("mount_point", ""));
    arguments.get_to(module_manager, "module_manager");
}

//-------------------------------------------------------------------------------------------------

void AssimpContainer::LoadFromMemory(StarVfs::FileResourceId source_resource_id, const std::string_view &file_data,
                                     StarVfs::HostFileSvfsManifest &file_manifest, const std::string_view &ext) {

    AddLog(FSEvent, "Loading assimp file from memory");

    auto loader = std::make_unique<AssimpLoader>(module_manager, source_resource_id, mount_point);
    if (!loader->Import(ext, file_data, file_manifest)) {
        throw std::runtime_error("Assimp import failed");
    }

    loaded_files.swap(loader->loaded_files);

    ReloadContainer();
}

void AssimpContainer::LoadFromCache(StarVfs::FileResourceId source_resource_id, const std::string_view &file_data,
                                    StarVfs::HostFileSvfsManifest &file_manifest, const std::string_view &ext) {

    AddLog(FSEvent, "Loading assimp file from cache");
    this->file_data = file_data;
    this->source_resource_id = source_resource_id;

    auto parent_id = StarVfs::Hasher::Hash(this->mount_point);

    loaded_files.clear();
    for (auto &item : file_manifest.sub_contents) {
        auto fi = std::make_unique<AssimpContainer::FileInfo>();

        auto resource_id = StarVfs::Hasher::Hash(item.file_name, source_resource_id);

        fi->resource_id = resource_id;
        fi->path_id = StarVfs::Hasher::Hash(StarVfs::JoinPath(mount_point, item.file_name));
        fi->container_id = fi->path_id;
        fi->parent_id = parent_id;

        fi->file_name = item.file_name;
        fi->file_data = "";
        loaded_files[fi->container_id] = std::move(fi);
    }

    ReloadContainer();
}

void AssimpContainer::ReloadContainer() {
    if (loaded_files.empty()) {
        return;
    }

    AddLog(FSEvent,
           fmt::format("Reloading assimp file mounted at {} object count: {}", mount_point, loaded_files.size()));

    std::vector<ContainerFileEntry> request_table;
    request_table.reserve(loaded_files.size());

    for (const auto &item : loaded_files) {
        ContainerFileEntry request_entry = {};
        request_entry.file_name = item.second->file_name;

        request_entry.container_file_id = item.first;
        request_entry.parent_path_hash = item.second->parent_id;
        request_entry.file_path_hash = item.second->path_id;
        request_entry.resource_id = item.second->resource_id;

        request_table.emplace_back(request_entry);
    }

    file_table_interface->CreateDirectory(mount_point);
    if (!file_table_interface->RegisterFileStructure(request_table)) {
        AddLog(Error, "Reloading assimp file failed");
    }
}

bool AssimpContainer::ReadFileContent(StarVfs::FilePathHash container_file_id, std::string &file_data) const {
    auto file_info = loaded_files.find(container_file_id);
    if (file_info == loaded_files.end()) {
        return false;
    }

    file_data = file_info->second->file_data;

    return true;
}

} // namespace MoonGlare::Tools::SvfsModules

#if 0
#include "AssimpImporter.h"

#include "../FileSystem.h"
#include <ToolBase/Module.h>
#include <iEditor.h>
#include <iFileIconProvider.h>
#include <iFileProcessor.h>
#include <icons.h>

#include <Common.x2c.h>
#include <DataModels/EditableEntity.h>
#include <Material.x2c.h>
#include <Math.x2c.h>
#include <Renderer/Configuration.Renderer.h>
#include <Renderer/Material.h>

namespace MoonGlare::Importer {

struct AssimpImporter : public QtShared::iFileProcessor, public QtShared::iEditor {

    AssimpImporter(SharedModuleManager modmgr, std::string URI)
        : QtShared::iFileProcessor(std::move(URI)), moduleManager(modmgr) {}

    bool OpenData(const std::string &URI) override {
        m_URI = URI;

        auto dotpos = URI.rfind(".");
        if (dotpos == std::string::npos || dotpos < URI.rfind("/")) {
            outputDirectory = URI + ".imported";
        } else {
            outputDirectory = URI.substr(0, dotpos);
        }

        sourceDirectory = std::filesystem::path(URI).remove_filename().generic_string();

        ProcessFile();
        return true;
    }

    void ImportBodyShapeComponent(const aiNode *node, EditableEntity *parent) {
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

    void ImportBodyComponent(const aiNode *node, EditableEntity *parent) {
        auto bodyC = parent->AddComponent("Body");
        auto bdio = bodyC->GetValuesEditor();
        bdio->Set("Kinematic", "1");
    }



    void ImportMeshComponent(const aiNode *node, EditableEntity *parent) {
        if (node->mNumMeshes != 1)
            return;

        auto meshSrc = scene->mMeshes[node->mMeshes[0]];
        if (!meshSrc->HasBones()) {
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

    void ImportLightComponent(EditableEntity *parent) {
        // auto it = sceneLights.find(parent->GetName());
        // if (it == sceneLights.end())
        //    return;
        // auto light = it->second;
        // auto lightC = parent->AddComponent(Core::SubSystemId::Light);
        // auto liio = lightC->GetValuesEditor();
    }

    void ImportEntityTree() {
        std::unique_ptr<QtShared::DataModels::EditableEntity> entity;

        entity = std::make_unique<EditableEntity>();

        std::vector<std::pair<EditableEntity *, const aiNode *>> queue;
        queue.emplace_back(entity.get(), scene->mRootNode);

        std::size_t cnt = 0;
        while (!queue.empty()) {
            ++cnt;
            auto item = queue.back();
            queue.pop_back();
            EditableEntity *parent = item.first;
            auto node = item.second;

            parent->SetName(node->mName.data);

            ImportTransformComponent(node, parent);
            ImportMeshComponent(node, parent);
            ImportLightComponent(parent);
            ImportBodyComponent(node, parent);
            ImportBodyShapeComponent(node, parent);

            for (int i = node->mNumChildren - 1; i >= 0; --i) {
                auto ch = node->mChildren[i];
                // if( ch->mNumMeshes == 1 )
                queue.emplace_back(parent->AddChild(), ch);
            }
        }

        entity->SetName(boost::filesystem::path(m_URI).filename().string());

        pugi::xml_document xdoc;
        if (!entity->Write(xdoc.append_child("Entity"))) {
            // AddLog(Hint, "Failed to write epx Entities: " << m_CurrentPatternFile);
            throw false;
        }
        generatedFiles["entity.epx"] = XmlToData(xdoc);
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
            } catch (...) {
            }
            return default;
        };

        auto getVec4Prop = [](aiMaterial *mat, const char *name) -> math::fvec4 {
            try {
                unsigned cnt = 4;
                float v[4] = {1, 1, 1, 1};
                if (mat->Get(name, 0, 0, v, &cnt) == aiReturn_SUCCESS) {
                    __nop();
                }
                return math::fvec4{v[0], v[1], v[2], v[3]};
            } catch (...) {
            }
            return {};
        };

        auto getVec3Prop = [](aiMaterial *mat, const char *name,
                              math::fvec3 def = {
                                  1,
                                  1,
                                  1,
                              }) -> math::fvec3 {
            try {
                unsigned cnt = 3;
                float v[3] = {1, 1, 1};
                if (mat->Get(name, 0, 0, v, &cnt) == aiReturn_SUCCESS) {
                    return math::fvec3{v[0], v[1], v[2]};
                }
            } catch (...) {
            }
            return def;
        };

        auto getStringProp = [](aiMaterial *mat, const char *name) -> std::string {
            try {
                if (aiString v; mat->Get(name, 0, 0, v) == aiReturn_SUCCESS) {
                    return v.data;
                }
                return "";
            } catch (...) {
            }
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
                        } else {
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
            // loadMap(aiTextureType_SPECULAR, matData.specularMap);
            loadMap(aiTextureType_NORMALS, matData.normalMap);
            loadMap(aiTextureType_SHININESS, matData.shinessMap);

            matData.diffuseColor = getVec3Prop(mat, "$clr.diffuse");
            matData.specularColor = getVec3Prop(mat, "$clr.specular");
            matData.emissiveColor = getVec3Prop(mat, "$clr.emissive",
                                                {
                                                    0,
                                                    0,
                                                    0,
                                                });
            matData.shiness = getFloaProp(mat, "$mat.shininess", 32);

            pugi::xml_document xdoc;
            x2c::Renderer::MaterialTemplate_t_Write(xdoc.append_child("Material"), matData, nullptr);

            std::string fname;
            std::string matName = getStringProp(mat, "?mat.name");
            if (matName.empty())
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
            auto fs = moduleManager->QueryModule<Editor::FileSystem>();
            StarVFS::ByteTable bt;
            if (!fs->GetFileData(m_URI, bt)) {
                // todo: log sth
                throw std::runtime_error("Unable to read file: " + m_URI);
            }
            if (bt.byte_size() == 0) {
                // todo: log sth
            }

            unsigned flags = aiProcess_JoinIdenticalVertices | /* aiProcess_PreTransformVertices | */
                             aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_SortByPType;

            scene = importer.ReadFileFromMemory(bt.get(), bt.byte_size(), flags, strrchr(m_URI.c_str(), '.'));

            if (!scene) {
                return ProcessResult::UnknownFailure;
            }

            // output

            GatherLights();
            ImportMeshes();
            ImportAnimations();
            ImportTextures();
            ImportMaterials();
            ImportEntities();
            ImportEntityTree();

            StoreResult();
        } catch (const std::exception &) {
            return ProcessResult::UnknownFailure;
        }
        return ProcessResult::Success;
    }

private:
    const aiScene *scene;
    Assimp::Importer importer;
    SharedModuleManager moduleManager;
    std::map<std::string, const aiLight *> sceneLights;
    std::set<std::string> skippedNodes;


    struct MaterialInfo {
        std::string name;
        std::string uri;
    };
    std::vector<MaterialInfo> material;

    std::vector<std::string> textureNames;

    std::string outputDirectory;
    std::string sourceDirectory;
    std::map<std::string, StarVFS::ByteTable> generatedFiles;
};

} // namespace MoonGlare::Importer

#endif
