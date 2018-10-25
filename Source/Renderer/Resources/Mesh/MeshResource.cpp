
#include "MeshResource.h"
#include "../ResourceManager.h"

#include "AssimpMeshLoader.h"
#include "VAOBuilder.h"

#include "../../Commands/MemoryCommands.h"

#include <Foundation/Resources/Blob/MeshBlob.h>

namespace MoonGlare::Renderer::Resources {
                                                                         
MeshManager::MeshManager(ResourceManager *Owner) : resourceManager(Owner) {
    assert(Owner);
    asyncLoader = resourceManager->GetLoader();
    assert(asyncLoader);

    allocationBitmap.ClearAllocation();
    deviceHandle.fill(Device::InvalidVAOHandle);
    mesh.fill({});
    meshData.fill({});

    generations.fill(1);//TODO: random?

    Conf::VAOBuffers vaob;
    vaob.fill(Device::InvalidBufferHandle);
    vaoBuffer.fill(vaob);
}

MeshManager::~MeshManager() {
}

//---------------------------------------------------------------------------------------

MeshResourceHandle MeshManager::Allocate() {
    Bitmap::Index_t index;
    if (allocationBitmap.Allocate(index)) {
       //if (/*queue &&*/ deviceHandle[index] == Device::InvalidTextureHandle) {
       //    //auto arg = queue->PushCommand<Commands::TextureSingleAllocate>();
       //    //arg->m_Out = &m_GLHandle[index];
       //}
        MeshResourceHandle hout;
        hout.index = static_cast<MeshResourceHandle::Index_t>(index);
        hout.generation = generations[index];
        hout.deviceHandle = &deviceHandle[index];
        return hout;
    }
    else {
        AddLogf(Debug, "mesh allocation failed");
        return {};
    }
}

std::pair<MeshResourceHandle, bool> MeshManager::Allocate(const std::string &uri) {
    auto cache = loadedMeshes.find(uri);
    if (cache != loadedMeshes.end() && IsHandleValid(cache->second)) {
        AddLogf(Performance, "mesh load cache hit");
        return { cache->second, true };
    }
    MeshResourceHandle hout = Allocate();
    loadedMeshes[uri] = hout;
    return { hout, false };
}

void MeshManager::Release(MeshResourceHandle hin) {
    if (!IsHandleValid(hin))
        return;

    hin.deviceHandle = nullptr;
    ++generations[hin.index];

    allocationBitmap.Release(hin.index);
}

bool MeshManager::IsHandleValid(MeshResourceHandle &h) const {
    if (h.index >= Conf::Limit)
        return false;
    if (generations[h.index] != h.generation) {
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------

void MeshManager::ApplyMeshSource(MeshResourceHandle h, MeshData source, std::unique_ptr<char[]> sourceMemory) {
    if (!IsHandleValid(h))
        return;

    auto index = h.index;
    source.ready = true;
    meshData[index] = source;
    mesh[index] = {};
    meshDataMemory[index].swap(sourceMemory);

    asyncLoader->PostTask([this, h, source](ResourceLoadStorage &storage) {
        auto index = h.index;
        Mesh m = { };

        m.valid = false;
        m.elementMode = GL_TRIANGLES;      
        m.indexElementType = GL_UNSIGNED_INT;
        m.baseVertex = 0;
        m.baseIndex = 0;
        m.numIndices = source.indexCount;         
        mesh[index] = m;     
        meshData[index].ready = false;

        auto &q = storage.m_Queue;
        auto builder = Builder::VAOBuilder{
            &q,
            &vaoBuffer[h.index][0],
            &deviceHandle[h.index],
        };

        Commands::CommandKey key = {};
        if (*builder.m_HandlePtr == Device::InvalidVAOHandle)
            q.MakeCommandKey<Commands::VAOSingleAllocate>(key, builder.m_HandlePtr);

        using ichannels = Renderer::Configuration::VAO::InputChannels;

        builder.BeginDataChange();

        if (source.verticles) {
            builder.CreateChannel(ichannels::Vertex);
            builder.SetChannelData<float, 3>(ichannels::Vertex, (const float*)(source.verticles), source.vertexCount);
        }

        if (source.UV0) {
            builder.CreateChannel(ichannels::Texture0);
            builder.SetChannelData<float, 2>(ichannels::Texture0, (const float*)(source.UV0), source.vertexCount);
        }

        if (source.normals) {
            builder.CreateChannel(ichannels::Normals);
            builder.SetChannelData<float, 3>(ichannels::Normals, (const float*)(source.normals), source.vertexCount);
        }     

        if (source.tangents) {
            builder.CreateChannel(ichannels::Tangents);
            builder.SetChannelData<float, 3>(ichannels::Tangents, (const float*)(source.tangents), source.vertexCount);
        }

        if (source.index) {
            builder.CreateChannel(ichannels::Index);
            builder.SetIndex(ichannels::Index, (const unsigned*)(source.index), source.indexCount);
        }

        builder.EndDataChange();
        builder.UnBindVAO();

        q.MakeCommand<Commands::MemoryStore<bool>>(true, &mesh[index].valid);
        q.MakeCommand<Commands::MemoryStore<bool>>(true, &meshData[index].ready);

#ifdef DEBUG
        SaveMeshObj(h);
        SaveMeshBin(h);
#endif 

    });
}

void MeshManager::ApplyMeshSource(MeshResourceHandle h, MeshSource source) {
    if (!IsHandleValid(h))
        return;

    size_t memorySize = 0;

    size_t verticlesSize = source.verticles.size() * sizeof(source.verticles[0]);
    size_t UV0Size = source.UV0.size() * sizeof(source.UV0[0]);
    size_t normalsSize = source.normals.size() * sizeof(source.normals[0]);
    size_t tangentsSize = source.tangents.size() * sizeof(source.tangents[0]);
    size_t indexSize = source.index.size() * sizeof(source.index[0]);

    size_t vertexBonesSize = source.vertexBones.size() * sizeof(source.vertexBones[0]);
    size_t vertexBoneWeightsSize = source.vertexBoneWeights.size() * sizeof(source.vertexBoneWeights[0]);
    size_t boneMatricesSize = source.boneOffsetMatrices.size() * sizeof(source.boneOffsetMatrices[0]);

    size_t boneNamesArraySize = source.boneNames.size() * sizeof(const char*);
    uint16_t boneNamesValuesSize = 0;
    std::for_each(source.boneNames.begin(), source.boneNames.end(), [&boneNamesValuesSize](auto &item) { boneNamesValuesSize += item.size() + 1; });

    size_t verticlesOffset = memorySize;
    memorySize += verticlesSize;
    size_t UV0Offset = memorySize;
    memorySize += UV0Size;
    size_t normalsOffset = memorySize;
    memorySize += normalsSize;
    size_t tangentsOffset = memorySize;
    memorySize += tangentsSize;
    size_t indexOffset = memorySize;
    memorySize += indexSize;        

    size_t vertexBonesOffset = memorySize;
    memorySize += vertexBonesSize;
    size_t vertexBoneWeightsOffset = memorySize;
    memorySize += vertexBoneWeightsSize;

    size_t boneNamesArrayOffset = memorySize;
    memorySize += boneNamesArraySize;
    size_t boneNamesValuesOffset = memorySize;
    memorySize += boneNamesValuesSize;

    size_t boneMatricesOffset = memorySize;
    memorySize += boneMatricesSize;

    std::unique_ptr<char[]> memory(new char[memorySize]);
    char *mem = memory.get();

    MeshData md = {};

    md.verticles = (glm::fvec3*)(mem + verticlesOffset);
    memcpy(md.verticles, &source.verticles[0], verticlesSize);

    if (source.UV0.size() > 0) {
        md.UV0 = (glm::fvec2*)(mem + UV0Offset);
        memcpy(md.UV0, &source.UV0[0], UV0Size);
    }

    if (source.normals.size() > 0) {
        md.normals = (glm::fvec3*)(mem + normalsOffset);
        memcpy(md.normals, &source.normals[0], normalsSize);
    }

    if (source.tangents.size() > 0) {
        md.tangents = (glm::fvec3*)(mem + tangentsOffset);
        memcpy(md.tangents, &source.tangents[0], tangentsSize);
    }

    md.index = (uint32_t*)(mem + indexOffset);
    memcpy(md.index, &source.index[0], indexSize);

    if (source.vertexBones.size() > 0) {
        md.vertexBones = (glm::u8vec4*)(mem + vertexBonesOffset);
        memcpy(md.vertexBones, &source.vertexBones[0], vertexBonesSize);
    }
    if (source.vertexBoneWeights.size() > 0) {
        md.vertexBoneWeights = (glm::fvec4*)(mem + vertexBoneWeightsOffset);
        memcpy(md.vertexBoneWeights, &source.vertexBoneWeights[0], vertexBoneWeightsSize);
    }
    if (source.boneNames.size() > 0) {
        md.boneNameValues = (const char*)(mem + boneNamesValuesOffset);
        md.boneNameOffsets = (uint16_t*)(mem + boneNamesArrayOffset);
        uint16_t offset = 0;
        for (size_t i = 0; i < source.boneNames.size(); ++i) {
            char * str = (char*)(mem + boneNamesValuesOffset + offset);
            md.boneNameOffsets[i] = offset;
            size_t len = source.boneNames[i].size();
            memcpy(str, source.boneNames[i].c_str(), len);
            str[len] = '\0';
            offset += len + 1;
        }
        assert(offset == boneNamesValuesSize);
    }
    if (source.boneOffsetMatrices.size() > 0) {
        md.boneMatrices = (glm::fmat4*)(mem + boneMatricesOffset);
        memcpy(md.boneMatrices, &source.boneOffsetMatrices[0], boneMatricesSize);
    }
    md.boneCount = source.boneNames.size();

    md.vertexCount = source.verticles.size();
    md.indexCount = source.index.size();

    md.halfBoundingBox = source.halfBoundingBox;
    md.boundingRadius = source.boundingRadius;
    md.memoryBlockSize = memorySize;
    md.memoryBlockFront = mem;
    md.ready = false;

#ifdef DEBUG_LOG
    {
        std::string uri = "<invalid>";
        for (auto &[cu, ch] : loadedMeshes)
            if (h == ch) {
                uri = cu;
                break;
            }
        auto b = [](auto v) -> char { return static_cast<bool>(v) ? 'T' : 'F'; };
        AddLogf(Resources, "Applied mesh source for '%s'; Memory:%5.1fkib Verts:%u Indices:%u Normals:%c UV0:%c Tangents:%c  Bones:%u", 
            uri.c_str(),
            memorySize / 1024.0f,
            md.vertexCount, md.indexCount,
            b(md.normals), b(md.UV0), b(md.tangents),
            md.boneCount
        );
    }
#endif

    ApplyMeshSource(h, md, std::move(memory));
}

MeshResourceHandle MeshManager::CreateMesh(MeshSource source, const std::string &uri) {
    auto [h, got] = Allocate(uri);
    ApplyMeshSource(h, source);
    return h;
}            

MeshResourceHandle MeshManager::LoadMesh(const std::string &uri) {
    auto[h, got] = Allocate(uri);
    if (got)
        return h;

    std::string subpath;
    std::string fileuri;

    auto pos = uri.find('@');
    if (pos == std::string::npos) {
        fileuri = uri;
    } else {
        fileuri = uri.substr(0, pos);
        subpath = uri.substr(pos + 1);
    }

    auto request = std::make_shared<Loader::AssimpMeshLoader>(std::move(subpath), h, *this);
    resourceManager->GetLoader()->QueueRequest(fileuri, request);

    return h;
}

//---------------------------------------------------------------------------------------

void MeshManager::SaveMeshObj(MeshResourceHandle h, std::string outFile) {
    if (outFile.empty())
        outFile = "logs/mesh." + std::to_string(h.index) + ".obj";

    std::ofstream of(outFile, std::ios::out);
    of << "#Mesh index:" << std::to_string(h.index) << " generation:" << std::to_string(h.generation) << "\n";
    for(auto &item : loadedMeshes)
        if (item.second.index == h.index && item.second.generation == h.generation) {
            of << "#Source: " << item.first <<  "\n";
            break;
        }
    of << "\n";

    auto *mptr = GetMeshData(h);
    if (!mptr)
        return;

    auto &m = *mptr;

    for (size_t i = 0; i < m.vertexCount; ++i) {
        auto &v = m.verticles[i];
        of << fmt::format("v {} {} {}\n", v[0], v[1], v[2]);
    }
    of << "\n";

    if (m.UV0) {
        for (size_t i = 0; i < m.vertexCount; ++i) {
            auto &v = m.UV0[i];
            of << fmt::format("vt {} {}\n", v[0], v[1]);
        }
        of << "\n";
    }

    if (m.normals) {
        for (size_t i = 0; i < m.vertexCount; ++i) {
            auto &v = m.normals[i];
            of << fmt::format("vn {} {} {}\n", v[0], v[1], v[2]);
        }
        of << "\n";
    }

    for (size_t i = 0; i < m.indexCount; i += 3) {
        auto *base = &m.index[i];
        of << fmt::format("f {0}/{0}/{0} {1}/{1}/{1} {2}/{2}/{2}\n", base[0] + 1, base[1] + 1, base[2] + 1);
    }

    of.close();
}

void MeshManager::SaveMeshBin(MeshResourceHandle h, std::string outFile) {
    if (outFile.empty())
        outFile = "logs/mesh." + std::to_string(h.index) + ".mesh";

    auto *mptr = GetMeshData(h);
    if (!mptr)
        return;

    std::ofstream of(outFile, std::ios::out | std::ios::binary);

    Blob::WriteMeshBlob(of, mptr);
}

} //namespace MoonGlare::Renderer::Resources 
