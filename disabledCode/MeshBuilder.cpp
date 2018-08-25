#include <pch.h>

#define NEED_MATERIAL_BUILDER
#define NEED_VAO_BUILDER
#define NEED_MESH_BUILDER

#include <MoonGlare.h>
#include "iModule.h"
#include <Foundation/Scripts/iLuaRequire.h>
#include <Core/Scripts/ScriptEngine.h>

#include <Renderer/Renderer.h>
#include <Renderer/Configuration/Mesh.h>
#include <Renderer/Resources/ResourceManager.h>
#include <Renderer/Resources/Mesh/MeshResource.h>

#include <Renderer/Resources/Mesh/MeshUpdate.h>

namespace MoonGlare::Modules {

struct MaterialInfo {
    std::vector<uint32_t> index;
    std::string TextureURI;

    Renderer::MaterialResourceHandle matHandle;
};

constexpr size_t StorageReserve = 1024;
struct MeshBuilder {

    MeshBuilder(World *world) : world(world) {
        verticles.reserve(StorageReserve);
        UVs.reserve(StorageReserve);
        normals.reserve(StorageReserve);
    }

    //bool weld = false;

    void AddTriangle(int MaterialIndex, int p0, int p1, int p2) {
        if (MaterialIndex < 0 || MaterialIndex >= materials.size()) {
            //TODO: some error
            return;
        }
        auto &mat = materials[MaterialIndex];
        mat.index.emplace_back(p0);
        mat.index.emplace_back(p1);
        mat.index.emplace_back(p2);
    }
    void AddQuad(int MaterialIndex, int p0, int p1, int p2, int p3) {
        if (MaterialIndex < 0 || MaterialIndex >= materials.size()) {
            //TODO: some error
            return;
        }
        auto &mat = materials[MaterialIndex];
        mat.index.emplace_back(p0);
        mat.index.emplace_back(p1);
        mat.index.emplace_back(p2);

        mat.index.emplace_back(p0);
        mat.index.emplace_back(p2);
        mat.index.emplace_back(p3);
    }

    int AddNode(const glm::fvec3 *position, const ::glm::fvec2 *UV, const ::glm::fvec3 *normal) {
        int idx = verticles.size();
        verticles.emplace_back(position ? *position : glm::fvec3(0));
        UVs.emplace_back(UV ? *UV : glm::fvec2(0));
        normals.emplace_back(normal ? *normal : glm::fvec3(0));
        return idx;
    }
    int AddMaterial(std::string MaterialURI) {
        int idx = materials.size();
        materials.emplace_back();
        auto &mat = materials.back();
        mat.TextureURI.swap(MaterialURI);
        mat.index.reserve(StorageReserve * 3);
        return idx;
    }

    void Dump(const std::string &name) {
        std::ofstream of("logs/matdump." + name + ".obj", std::ios::out);
        of << "\n";

        for (auto &v : verticles)
            of << fmt::format("v {} {} {}\n", v[0], v[1], v[2]);
        of << "\n";

        for (auto &v : UVs)
            of << fmt::format("vt {} {}\n", v[0], v[1]);
        of << "\n";

        for (auto &v : normals)
            of << fmt::format("vn {} {} {}\n", v[0], v[1], v[2]);
        of << "\n";

        for (auto &mat : materials) {
            of << fmt::format("# material uri: {}\n", mat.TextureURI);
            for (size_t i = 0; i < mat.index.size(); i += 3) {
                auto *base = &mat.index[i];
                of << fmt::format("f {0}/{0}/{0} {1}/{1}/{1} {2}/{2}/{2}\n", base[0]+1, base[1]+1, base[2]+1);
            }
        }

        of.close();
    }

    Renderer::MeshResourceHandle GetHandle() {
        auto *rf = world->GetRendererFacade();
        auto *resmgr = rf->GetResourceManager();
        auto &mm = resmgr->GetMeshManager();
        if (!mm.IsHandleValid(handle)) {
            mm.Allocate(handle);
        }

        return handle;
    }

    void Update() {
#if 0
        if (verticles.empty())
            return;

        auto *rf = world->GetRendererFacade();
        auto *resmgr = rf->GetResourceManager();
        auto &mm = resmgr->GetMeshManager();
        auto &matm = resmgr->GetMaterialManager();

        Renderer::Configuration::Mesh::SubMesh meshArray = {};
        Renderer::MaterialResourceHandle materialArray = {};

        std::vector<uint32_t> meshIndex;

        for (size_t index = 0; index < materials.size(); ++index) {
            auto &matInfo = materials[index];

            if (!matm.IsHandleValid(matInfo.matHandle)) {
                //load material
                auto matb = matm.GetMaterialBuilder(matInfo.matHandle, true);
                matb.SetDiffuseMap(matInfo.TextureURI);
                matb.SetDiffuseColor(emath::fvec4(1,1,1,1));
            }

            materialArray[index] = matInfo.matHandle;

            auto &mesh = meshArray[index];
            mesh.valid = true;
            mesh.baseVertex = 0;
            mesh.baseIndex = meshIndex.size() *sizeof(uint32_t);
            mesh.elementMode = GL_TRIANGLES;
            mesh.indexElementType = GL_UNSIGNED_INT;
            mesh.numIndices = matInfo.index.size();

            meshIndex.reserve(matInfo.index.size());
            for (auto itm: matInfo.index) {
                meshIndex.emplace_back(itm);
            }
        }
        if (meshIndex.empty())
            return;

        auto h = GetHandle();
        Renderer::Resources::MeshData md;
        md.index.swap(meshIndex);
        md.verticles = verticles;
        md.normals = normals;
        md.UV0 = UVs;
        md.UpdateBoundary();
        mm.SetMeshData(h, std::move(md));

        auto task = std::make_shared<Renderer::Resources::Loader::CustomMeshLoader>(h, mm);
        task->materialArray = materialArray;
        task->meshArray = meshArray;
        rf->GetAsyncLoader()->QueueTask(task);
#endif
    }

    void Clear() {
        verticles.clear();
        UVs.clear();
        normals.clear();

        auto *rf = world->GetRendererFacade();
//        auto *resmgr = rf->GetResourceManager();
//        auto &matm = resmgr->GetMaterialManager();
        for (auto &mat : materials) {             
            //matm.Release(mat.matHandle);
            //TODO: release materials
        }

        materials.clear();
    }
private:
    Renderer::MeshResourceHandle handle;

    std::vector<::glm::fvec3> verticles;
    std::vector<::glm::fvec2> UVs;
    std::vector<::glm::fvec3> normals;

    std::vector<MaterialInfo> materials;

    World * world = nullptr;
};

struct MeshBuilderModule : public MoonGlare::Modules::iModule, public Core::Scripts::iRequireRequest {
    MeshBuilderModule(World *world) : iModule(world) {}

    void OnPostInit() override {
        auto smod = Core::GetScriptEngine()->QuerryModule<Core::Scripts::iRequireModule>();
        RegisterApi(Core::GetScriptEngine()->GetLua());
        smod->RegisterRequire("MeshBuilder", this);
    }

    bool OnRequire(lua_State *lua, std::string_view name) override {
        luabridge::push(lua, this);
        return true;
    }

    MeshBuilder CreateBuilder() {
        return MeshBuilder(GetWorld());
    }

    void RegisterApi(lua_State *lua) {
        luabridge::getGlobalNamespace(lua)
            .beginNamespace("api")
                .beginClass<MeshBuilderModule>("cMeshBuilderModule")
                    .addFunction("New", &MeshBuilderModule::CreateBuilder)
                .endClass()
                .beginClass<MeshBuilder>("cMeshBuilder")
                    .addFunction("AddMaterial", &MeshBuilder::AddMaterial)
                    .addFunction("AddNode", &MeshBuilder::AddNode)

                    .addFunction("AddTriangle", &MeshBuilder::AddTriangle)
                    .addFunction("AddQuad", &MeshBuilder::AddQuad)

                    .addFunction("GetHandle", &MeshBuilder::GetHandle)
                    .addFunction("Update", &MeshBuilder::Update)
                    .addFunction("Clear", &MeshBuilder::Clear)

#ifdef DEBUG_SCRIPTAPI
                    .addFunction("Dump", &MeshBuilder::Dump)
#endif
                .endClass()
            .endNamespace()
            ;
    }
};

static ModuleClassRegister::Register<MeshBuilderModule> Reg("MeshBuilder");

} //namespace MoonGlare::Modules
