#include <Foundation/Memory/AlignedPtr.h>

#include "AssimpAnimationLoader.h"

namespace MoonGlare::Resources::Loader {

void AssimpAnimationLoader::OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata) {
    importer = std::make_unique<Assimp::Importer>();

    auto loadflags =
        //aiProcessPreset_TargetRealtime_Fast | 
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace |
        aiProcess_GenNormals |
        aiProcess_LimitBoneWeights |
        //aiProcess_ImproveCacheLocality |
        //aiProcess_PreTransformVertices |
        aiProcess_Triangulate |
        aiProcess_GenUVCoords |
        aiProcess_SortByPType |
        aiProcess_GlobalScale |
        0;

    scene = importer->ReadFileFromMemory(filedata.get(), filedata.size(), loadflags, strrchr(requestedURI.c_str(), '.'));

    if (!scene) {
        AddLog(Error, fmt::format("Unable to animation model file[{}]. Error: {}", requestedURI, importer->GetErrorString()));
        return;
    }

    ModelURI = requestedURI;
    baseURI = requestedURI;
    baseURI.resize(baseURI.rfind('/') + 1);

    LoadFile(requestedURI + ".xml", [this](const std::string&, StarVFS::ByteTable &filedata) {
        if (!animSetXmlDoc.load_string(filedata.c_str())) {
            //TODO: sth
            __debugbreak();
        }
        LoadAnimation();
    });
}

int AssimpAnimationLoader::GetAnimIndex() const {
    static constexpr std::string_view meshProto = "animation://";

    if (subpath.empty()) {
        return 0;
    }

    if (subpath.find(meshProto) != 0) {
        __debugbreak();
        return -1;
    }

    const char *beg = subpath.c_str() + meshProto.size();

    if (*beg == '*') {
        ++beg;
        char *end = nullptr;
        long r = strtol(beg, &end, 10);
        if (end == beg) {
            __debugbreak();
            return -1;
        }
        if (r > (int)scene->mNumMeshes)
            return -1;
        return r;
    }

    std::string name(beg);
    for (unsigned i = 0; i < scene->mNumMeshes; i++) {
        auto mesh = scene->mMeshes[i];
        if (name == mesh->mName.data) {
            return i;
        }
    }

    __debugbreak();
    return -1;
}

void AssimpAnimationLoader::LoadAnimation() {
    int animId = GetAnimIndex();
    if (animId < 0) {
        __debugbreak();
        return;
    }

    const auto *assimpAnim = scene->mAnimations[animId];
    pugi::xml_node animSetNode = animSetXmlDoc.document_element().find_child_by_attribute("AnimationSet", "Index", std::to_string(animId).c_str());

    //calculate memory requirement:
    size_t AnimationDataSize = 0;
    size_t stringArraySize = 1; //first byte to be zero  
    stringArraySize += assimpAnim->mName.length + 1;     

    for (size_t ch = 0; ch < assimpAnim->mNumChannels; ++ch) {
        size_t chsize = 0;
        auto &channel = assimpAnim->mChannels[ch];
        chsize += Memory::Align16(sizeof(PositionKey) * channel->mNumPositionKeys);
        chsize += Memory::Align16(sizeof(ScalingKey)  * channel->mNumScalingKeys );
        chsize += Memory::Align16(sizeof(RotationKey) * channel->mNumRotationKeys);
        stringArraySize += channel->mNodeName.length + 1;
        AnimationDataSize += chsize;
    }

    if (animSetNode) {
        for (auto node = animSetNode.first_child(); node; node = node.next_sibling()) {
            std::string n = node.attribute("Name").as_string("");
            stringArraySize += n.size() + 1;
        }
    }

    size_t dataAllocOffset = Memory::Align16(stringArraySize);
    AnimationDataSize += dataAllocOffset;

    std::unique_ptr<char[]> memory(new char[AnimationDataSize]);
    char *mem = memory.get();
    memset(mem, 0, AnimationDataSize);
    SkeletalAnimation animInfo = {};

    animInfo.stringArrayBase = (char*)mem;

    size_t stringAllocOffset = 1;

    auto pushString = [&stringAllocOffset, &animInfo](const char* text) {
        size_t len = strlen(text);
        size_t offset = stringAllocOffset;
        stringAllocOffset += len + 1;
        char *out = (char*)animInfo.stringArrayBase + offset;
        strcpy(out, text);
        out[len] = '\0';
        return offset;
    };

    auto allocData = [&dataAllocOffset, mem](auto arr, size_t cnt) -> decltype(arr) {
        size_t elemSize = sizeof(arr[0]);
        size_t len = cnt * elemSize;
        len = Memory::Align16(len);
        size_t offset = dataAllocOffset;
        dataAllocOffset = Memory::Align16(dataAllocOffset + len);
        assert((dataAllocOffset & 0xF) == 0);
        return reinterpret_cast<decltype(arr)>(mem + offset);
    };

    animInfo.animationNameOffset = pushString(assimpAnim->mName.data);

    if (animSetNode) {
        size_t index = 0;
        for (auto node = animSetNode.first_child(); node; node = node.next_sibling()) {
            animInfo.animationSetNameOffset[index] = pushString(node.attribute("Name").as_string(""));
            auto start = node.attribute("Start").as_uint();
            auto end = node.attribute("End").as_uint();
            auto loop = node.attribute("Loop").as_bool();

            auto &set = animInfo.animationSet[index];
            set.firstFrame = start;
            set.endFrame = end;
            set.loop = loop ? 1 : 0;
            ++index;
        }
        animInfo.animationSetCount = index;
    }

    animInfo.channelCount = assimpAnim->mNumChannels;
    for (size_t ch = 0; ch < assimpAnim->mNumChannels; ++ch) {
        auto &assimpChannel = assimpAnim->mChannels[ch];
        animInfo.channelNameOffset[ch] = pushString(assimpChannel->mNodeName.data);
        auto &channel = animInfo.channel[ch];

        channel.positionKeyCount = assimpChannel->mNumPositionKeys;
        channel.rotationKeyCount = assimpChannel->mNumRotationKeys;
        channel.scalingKeyCount = assimpChannel->mNumScalingKeys;

        channel.positionKey = allocData(channel.positionKey, channel.positionKeyCount);
        for (size_t key = 0; key < channel.positionKeyCount; ++key) {
            auto &k = channel.positionKey[key];
            const auto &v = assimpChannel->mPositionKeys[key].mValue;
            k.time = assimpChannel->mPositionKeys[key].mTime;
            k.value = { v.x, v.y, v.z };
        }

        channel.rotationKey = allocData(channel.rotationKey, channel.rotationKeyCount);
        for (size_t key = 0; key < channel.rotationKeyCount; ++key) {
            auto &k = channel.rotationKey[key];
            const auto &v = assimpChannel->mRotationKeys[key].mValue;
            k.time = assimpChannel->mRotationKeys[key].mTime;
            if constexpr (std::is_same_v<RotationKey::item_t, emath::Quaternion>) {
                k.value = { v.w, v.x, v.y, v.z };
            } else {
                if constexpr (std::is_same_v<RotationKey::item_t, glm::fvec3>) {
                    k.value = { v.x, v.y, v.z, v.w };
                } else {
                    __debugbreak();
                }
            }
        }

        channel.scalingKey = allocData(channel.scalingKey, channel.scalingKeyCount);
        for (size_t key = 0; key < channel.scalingKeyCount; ++key) {
            auto &k = channel.scalingKey[key];
            const auto &v = assimpChannel->mScalingKeys[key].mValue;
            k.time = assimpChannel->mScalingKeys[key].mTime;
            k.value = { v.x, v.y, v.z };
        }
    }

    assert(stringAllocOffset == stringArraySize);
    assert(AnimationDataSize == dataAllocOffset);

    animInfo.duration = assimpAnim->mDuration;
    animInfo.ticksPerSecond = assimpAnim->mTicksPerSecond;

    animInfo.memoryBlockFront = mem;
    animInfo.memoryBlockSize = AnimationDataSize;
    owner.ApplyAnimationData(handle, std::move(memory), animInfo);

#if 0
    uint32_t NumVertices = 0, NumIndices = 0;
       
    struct LoadInfo {
        size_t baseIndex;
    };

    Mesh meshes = {};
    LoadInfo loadInfo;

    {
        auto mesh = scene->mMeshes[meshId];
        meshes.valid = true;
        meshes.numIndices = mesh->mNumFaces * 3;
        meshes.baseVertex = static_cast<uint16_t>(NumVertices);
        meshes.baseIndex = static_cast<uint16_t>(NumIndices * sizeof(uint32_t));
        meshes.elementMode = GL_TRIANGLES;
        meshes.indexElementType = GL_UNSIGNED_INT;
        loadInfo.baseIndex = NumIndices;
        NumVertices = mesh->mNumVertices;
        NumIndices = meshes.numIndices;
    }

    MeshSource meshData;
    meshData.verticles.resize(NumVertices);
    meshData.UV0.resize(NumVertices);
    meshData.normals.resize(NumVertices);
    meshData.tangents.resize(NumVertices);
    meshData.index.resize(NumIndices);

    const aiMesh* mesh = scene->mMeshes[meshId];
    {
        auto MeshVerticles = &meshData.verticles[meshes.baseVertex];
        auto MeshTexCords = &meshData.UV0[meshes.baseVertex];
        auto MeshNormals = &meshData.normals[meshes.baseVertex];
        auto MeshTangents = &meshData.tangents[meshes.baseVertex];

        for (size_t vertid = 0; vertid < mesh->mNumVertices; vertid++) {
            aiVector3D &vertex = mesh->mVertices[vertid];
            aiVector3D &normal = mesh->mNormals[vertid];

            if (mesh->mTextureCoords[0]) {
                aiVector3D &UVW = mesh->mTextureCoords[0][vertid]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
                MeshTexCords[vertid] = glm::fvec2(UVW.x, UVW.y);
            } else {
                MeshTexCords[vertid] = glm::fvec2();
            }
            if (mesh->mTangents) {
                aiVector3D &tangent = mesh->mTangents[vertid];
                MeshTangents[vertid] = glm::fvec3(tangent.x, tangent.y, tangent.z);
            }

            MeshVerticles[vertid] = glm::fvec3(vertex.x, vertex.y, vertex.z);
            MeshNormals[vertid] = glm::fvec3(normal.x, normal.y, normal.z);
        }

        auto meshIndices = &meshData.index[loadInfo.baseIndex];
        for (size_t face = 0; face < mesh->mNumFaces; face++) {
            aiFace *f = &mesh->mFaces[face];
            THROW_ASSERT(f->mNumIndices == 3, 0);
            meshIndices[face * 3 + 0] = f->mIndices[0];
            meshIndices[face * 3 + 1] = f->mIndices[1];
            meshIndices[face * 3 + 2] = f->mIndices[2];
        }
    }

    if (mesh->HasBones()) {
        meshData.vertexBones.resize(NumVertices, MeshSource::InvalidBoneIndexSlot());
        meshData.vertexBoneWeights.resize(NumVertices);
        meshData.boneNames.resize(mesh->mNumBones);
        meshData.boneOffsetMatrices.resize(mesh->mNumBones);

        auto AllocLocalBoneIndex = [](size_t index, const glm::u8vec4 &vec) -> uint8_t {
            for (uint8_t i = 0; i < 4; ++i)
                if (vec[i] == MeshSource::InvalidBoneIndex)
                    return i;
            return MeshSource::InvalidBoneIndex;
        };

        unsigned BaseVertex = 0;// meshData.BaseVertex;
        for (size_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
            const auto *bone = mesh->mBones[boneIndex];
            meshData.boneNames[boneIndex] = bone->mName.data;
            meshData.boneOffsetMatrices[boneIndex] = glm::transpose(*(glm::fmat4*)&bone->mOffsetMatrix);

            std::string vs;
            for (unsigned k = 0; k < bone->mNumWeights; ++k) {

                auto &VertexWeight = bone->mWeights[k];

                auto vertexid = BaseVertex + VertexWeight.mVertexId;
                auto & vertexBones = meshData.vertexBones[vertexid];
                vs += std::to_string(vertexid) + " ";
                auto localboneid = AllocLocalBoneIndex(vertexid, vertexBones);
                if (localboneid == MeshSource::InvalidBoneIndex) {
                    __debugbreak();
                    AddLogf(Warning, "Vertex %d cannot have more than 4 bones [%s]; Skipped influence of bone %s with weight %f", vertexid, subpath.c_str(), meshData.boneNames[boneIndex].c_str(), VertexWeight.mWeight);
                    continue;
                }

                meshData.vertexBones[vertexid][localboneid] = boneIndex;
                meshData.vertexBoneWeights[vertexid][localboneid] = VertexWeight.mWeight;
            }
        }
    }
#endif
}

} //namespace MoonGlare::Renderer::Resources::Loader 
