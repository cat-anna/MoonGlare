#pragma once

namespace MoonGlare::Renderer {

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
}
