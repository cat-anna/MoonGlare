#pragma once

#include <Math/libMath.h>

namespace MoonGlare::Resources {

struct alignas(16) Mesh { 
    bool valid;
    uint8_t __padding[3];

    uint16_t elementMode;            //TODO: is this needed, is GL_TRIANGLES required? (no?)
    uint16_t indexElementType;

    uint16_t baseVertex;
    uint16_t baseIndex;

    uint32_t numIndices;
};
static_assert(sizeof(Mesh) == 16);
static_assert(std::is_pod_v<Mesh>);

struct MeshSource {
    std::vector<glm::fvec3> verticles;
    std::vector<glm::fvec2> UV0;
    std::vector<glm::fvec3> normals;
    std::vector<glm::fvec3> tangents;
    std::vector<uint32_t> index;

    static constexpr uint8_t InvalidBoneIndex = 0xFF;
    static glm::u8vec4 InvalidBoneIndexSlot() { return glm::u8vec4(0xFF); };
    std::vector<glm::u8vec4> vertexBones;
    std::vector<glm::fvec4> vertexBoneWeights;
    std::vector<std::string> boneNames;
    std::vector<glm::mat4> boneOffsetMatrices;

    emath::fvec3 halfBoundingBox;
    float boundingRadius;

    void UpdateBoundary() {
        boundingRadius = 0;
        halfBoundingBox = emath::fvec3(0, 0, 0);
        for (auto &v : verticles) {
            boundingRadius = std::max(boundingRadius, glm::length(v));
            for (int j = 0; j < 3; ++j)
                halfBoundingBox[j] = std::max(halfBoundingBox[j], abs(v[j]));
        }
    }
};

struct MeshData {
    glm::fvec3 *verticles;
    glm::fvec2 *UV0;
    glm::fvec3 *normals;
    glm::fvec3 *tangents;
    uint32_t *index;

    static constexpr uint8_t InvalidBoneIndex = MeshSource::InvalidBoneIndex;
    glm::u8vec4 *vertexBones;
    glm::fvec4 *vertexBoneWeights;
    glm::fmat4 *boneMatrices;
    const char *boneNameValues;
    uint16_t *boneNameOffsets;
    uint8_t boneCount;
    uint8_t __padding[3];

    uint32_t vertexCount;
    uint32_t indexCount;

    emath::fvec3 halfBoundingBox;
    float boundingRadius;

    bool ready;
    uint32_t memoryBlockSize;
    void *memoryBlockFront; //from this pointer all other should be relative within range of memoryBlockSize

    bool CheckPointers() const {
        auto *memEnd = ((const uint8_t*)memoryBlockFront) + memoryBlockSize;
        auto check = [this, memEnd] (const auto *ptr) -> bool {
            auto *p = (const uint8_t*)ptr;
            return ptr == nullptr || (p >= (const uint8_t*)memoryBlockFront && p < memEnd);
        };

        return 
            check(verticles) &&
            check(UV0) &&
            check(normals) &&
            check(tangents) &&
            check(index) &&
            check(vertexBones) &&
            check(vertexBoneWeights) &&
            check(boneMatrices) &&
            check(boneNameValues) &&
            check(boneNameOffsets);
    }
};

}
