#pragma once

#include <libSpace/src/Container/StaticVector.h>

#include <Core/Component/TemplateStandardComponent.h>
#include <Core/Scripts/ScriptComponent.h>

#pragma warning ( push, 0 )
#include <assimp/Importer.hpp>     
#include <assimp/scene.h>          
#include <assimp/postprocess.h>  
#pragma warning ( pop )

namespace MoonGlare {
namespace Component {
using namespace MoonGlare::Core::Component;

class TransformComponent;

struct DirectAnimationComponentEntry {
    union FlagsMap {
        struct MapBits_t {
            bool m_Valid : 1; //Entity is not valid or requested to be deleted;
            bool m_AnimValid : 1;
            bool m_Visible : 1;
            bool m_Loop : 1;
            bool m_Playing : 1;
        };
        MapBits_t m_Map;
        uint32_t m_UintValue;

        void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
        void ClearAll() { m_UintValue = 0; }

        static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
    };

    FlagsMap m_Flags;
    Entity m_Owner;
//	Handle m_MeshHandle;
//	DataClasses::ModelPtr m_Model;
//	std::string m_ModelName;

    const aiScene* m_Scene;

    struct MeshData /*: public Graphic::VAO::MeshData */{
        uint32_t BaseVertex, NumIndices, BaseIndex, ElementMode;

        uint32_t BoneCount;
        uint32_t VertexCount;
    };

    using MeshDataVector = std::vector<MeshData>;
    MeshDataVector m_Meshes;
    Renderer::VAOResourceHandle vaoHandle{};
    std::vector<math::fvec3> m_Mesh, m_RunTimeMesh;
    std::vector<math::fvec2> m_uvs;
    std::vector<unsigned> m_indices;
    std::vector<math::fvec3> m_MeshNormals, m_RunTimeMeshNormals;
    std::vector<math::uvec4> m_BoneIds;
    std::vector<math::fvec4> m_BoneWeights;

    uint8_t validBones = 0;
    Entity bones[64];
    std::vector<glm::mat4> boneOffsets;

    uint32_t m_FirstFrame, m_EndFrame;
    float m_FramesPerSecond;
    float m_LocalTime;

    bool IsVisible() const { return m_Flags.m_Map.m_Visible; }
    void SetVisible(bool v) { m_Flags.m_Map.m_Visible = v; }

    void Load() {
        // build the nodes-for-bones table
        for (unsigned int i = 0; i < m_Scene->mNumMeshes; ++i) {
            const aiMesh* mesh = m_Scene->mMeshes[i];
            for (unsigned int n = 0; n < mesh->mNumBones; ++n) {
                const aiBone* bone = mesh->mBones[n];

                m_BoneNodesByName[bone] = m_Scene->mRootNode->FindNode(bone->mName);
            }
        }

        m_RootNode = CreateNodeTree(m_Scene->mRootNode, NULL);
        m_Anim = m_Scene->mAnimations[m_CurrentAnimIndex];
        m_LastTime = 0.0;
        m_LastPositions.resize(m_Anim->mNumChannels, std::make_tuple(0, 0, 0));
    }

    void Reset() {
        m_Flags.m_Map.m_Valid = false;
        m_LocalTime = 0;
        m_FirstFrame = m_EndFrame = 0;
        m_FramesPerSecond = 0;
    }

 //------------------------------------------------

    struct AnimNode {
        std::string m_Name;
        AnimNode* m_Parent = nullptr;
        std::vector<std::unique_ptr<AnimNode>> m_Children;

        aiMatrix4x4 m_LocalTransform;
        aiMatrix4x4 m_GlobalTransform;

        size_t m_ChannelIndex = -1;

        AnimNode() { }
        AnimNode(const std::string& pName) : m_Name(pName) { }
    };

    struct BoneNode {

    };


    size_t m_CurrentAnimIndex = 0;
protected:

    std::unique_ptr<AnimNode> m_RootNode;
    std::vector<BoneNode> m_BoneNode;

    typedef std::map<const aiNode*, AnimNode*> NodeMap;
    NodeMap m_NodesByName;

    typedef std::map<const aiBone*, const aiNode*> BoneMap;
    BoneMap m_BoneNodesByName;

    aiMatrix4x4 m_IdentityMatrix;

    const aiAnimation* m_Anim;
public:
    void Calculate(double pTime) {
        Evaluate(pTime);
        UpdateTransforms(m_RootNode.get(), m_Transformations);
    }

    const aiMatrix4x4& GetLocalTransform(const aiNode* node) const {
        auto it = m_NodesByName.find(node);
        if (it == m_NodesByName.end())
            return m_IdentityMatrix;

        return it->second->m_LocalTransform;
    }

    const aiMatrix4x4& GetGlobalTransform(const aiNode* node) const {
        auto it = m_NodesByName.find(node);
        if (it == m_NodesByName.end())
            return m_IdentityMatrix;

        return it->second->m_GlobalTransform;
    }
    const aiMatrix4x4& GetGlobalTransform(const aiBone* bone) {
        return GetGlobalTransform(m_BoneNodesByName[bone]);
    }

    std::vector<aiMatrix4x4> m_Transforms;
    const std::vector<aiMatrix4x4>&  GetBoneMatrices(const aiNode* pNode, size_t pMeshIndex /* = 0 */) {
        size_t meshIndex = pNode->mMeshes[pMeshIndex];
        const aiMesh* mesh = m_Scene->mMeshes[meshIndex];

        m_Transforms.resize(std::max(mesh->mNumBones, 60u), aiMatrix4x4());

        //aiMatrix4x4 globalInverseMeshTransform = GetGlobalTransform(pNode);
        //globalInverseMeshTransform.Inverse();

        for (size_t a = 0; a < mesh->mNumBones; ++a) {
            const aiBone* bone = mesh->mBones[a];
            const aiMatrix4x4& currentGlobalTransform = GetGlobalTransform(bone);
            m_Transforms[a] = 
                //globalInverseMeshTransform *
                currentGlobalTransform
                //* bone->mOffsetMatrix
                ;
        }

        return m_Transforms;
    }

    std::unique_ptr<AnimNode> CreateNodeTree(aiNode* pNode, AnimNode* pParent) {
        // create a node
        auto internalNode = std::make_unique<AnimNode>(pNode->mName.data);
        internalNode->m_Parent = pParent;
        m_NodesByName[pNode] = internalNode.get();

        // copy its transformation
        internalNode->m_LocalTransform = pNode->mTransformation;
        CalculateGlobalTransform(internalNode.get());

        // find the index of the animation track affecting this node, if any
        if (m_CurrentAnimIndex < m_Scene->mNumAnimations) {
            internalNode->m_ChannelIndex = -1;
            const aiAnimation* currentAnim = m_Scene->mAnimations[m_CurrentAnimIndex];
            for (unsigned int a = 0; a < currentAnim->mNumChannels; a++) {
                if (currentAnim->mChannels[a]->mNodeName.data == internalNode->m_Name) {
                    internalNode->m_ChannelIndex = a;
                    break;
                }
            }
        }

        for (unsigned int a = 0; a < pNode->mNumChildren; a++) {
            internalNode->m_Children.emplace_back(CreateNodeTree(pNode->mChildren[a], internalNode.get()));
        }

        return std::move(internalNode);
    }

    void UpdateTransforms(AnimNode* pNode, const std::vector<aiMatrix4x4>& pTransforms) {
        if (pNode->m_ChannelIndex != -1) {
            pNode->m_LocalTransform = pTransforms[pNode->m_ChannelIndex];
        }

        CalculateGlobalTransform(pNode);

        for (auto &node: pNode->m_Children)
            UpdateTransforms(node.get(), pTransforms);
    }


    void CalculateGlobalTransform(AnimNode* pInternalNode) {
        pInternalNode->m_GlobalTransform = pInternalNode->m_LocalTransform;
        AnimNode* node = pInternalNode->m_Parent;
        while (node) {
            pInternalNode->m_GlobalTransform = node->m_LocalTransform * pInternalNode->m_GlobalTransform;
            node = node->m_Parent;
        }
    }

    double m_LastTime;
    std::vector<std::tuple<unsigned int, unsigned int, unsigned int> > m_LastPositions;
    std::vector<aiMatrix4x4> m_Transformations;
    struct LocalTransform {
        glm::fvec3 scale, position;
        glm::fvec4 quaternion;
        glm::mat4 mat, offset;       

    };
    std::vector<LocalTransform> m_LocalTransformations;
    std::vector<std::string> boneNames;
    std::unordered_map<std::string, size_t> boneIndexes;

    void Evaluate(double pTime) {
        // extract ticks per second. Assume default value if not given
        //double ticksPerSecond = m_Anim->mTicksPerSecond != 0.0 ? m_Anim->mTicksPerSecond : 25.0;
        // every following time calculation happens in ticks
    //	pTime *= ticksPerSecond;

        // map into anim's duration
        double time = 0.0f;
        if (m_Anim->mDuration > 0.0)
            time = fmod(pTime, m_Anim->mDuration);

        if (m_Transformations.size() != m_Anim->mNumChannels) {
            m_Transformations.resize(m_Anim->mNumChannels);
            m_LocalTransformations.resize(m_Anim->mNumChannels);
            boneNames.resize(m_Anim->mNumChannels);
            for (unsigned int a = 0; a < m_Anim->mNumChannels; a++) {
                boneNames[a] = m_Anim->mChannels[a]->mNodeName.data;
                boneIndexes[boneNames[a]] = a;
            }
        }

        for (unsigned int a = 0; a < m_Anim->mNumChannels; a++) {
            const aiNodeAnim* channel = m_Anim->mChannels[a];

            // ******** Position *****
            aiVector3D presentPosition(0, 0, 0);
            if (channel->mNumPositionKeys > 0) {
                unsigned int frame = (time >= m_LastTime) ? std::get<0>(m_LastPositions[a]) : 0;
                while (frame < channel->mNumPositionKeys - 1) {
                    if (time < channel->mPositionKeys[frame + 1].mTime)
                        break;
                    frame++;
                }

                unsigned int nextFrame = (frame + 1) % channel->mNumPositionKeys;
                const aiVectorKey& key = channel->mPositionKeys[frame];
                const aiVectorKey& nextKey = channel->mPositionKeys[nextFrame];
                double diffTime = nextKey.mTime - key.mTime;
                if (diffTime < 0.0)
                    diffTime += m_Anim->mDuration;
                if (diffTime > 0) {
                    float factor = float((time - key.mTime) / diffTime);
                    presentPosition = key.mValue + (nextKey.mValue - key.mValue) * factor;
                } else {
                    presentPosition = key.mValue;
                }

                std::get<0>(m_LastPositions[a]) = frame;
            }

            // ******** Rotation *********
            aiQuaternion presentRotation(1, 0, 0, 0);
            if (channel->mNumRotationKeys > 0) {
                unsigned int frame = (time >= m_LastTime) ? std::get<1>(m_LastPositions[a]) : 0;
                while (frame < channel->mNumRotationKeys - 1) {
                    if (time < channel->mRotationKeys[frame + 1].mTime)
                        break;
                    frame++;
                }

                unsigned int nextFrame = (frame + 1) % channel->mNumRotationKeys;
                const aiQuatKey& key = channel->mRotationKeys[frame];
                const aiQuatKey& nextKey = channel->mRotationKeys[nextFrame];
                double diffTime = nextKey.mTime - key.mTime;
                if (diffTime < 0.0)
                    diffTime += m_Anim->mDuration;
                if (diffTime > 0) {
                    float factor = float((time - key.mTime) / diffTime);
                    aiQuaternion::Interpolate(presentRotation, key.mValue, nextKey.mValue, factor);
                } else {
                    presentRotation = key.mValue;
                }

                std::get<1>(m_LastPositions[a]) = frame;
            }

            // ******** Scaling **********
            aiVector3D presentScaling(1, 1, 1);
            if (channel->mNumScalingKeys > 0) {
                unsigned int frame = (time >= m_LastTime) ? std::get<2>(m_LastPositions[a]) : 0;
                while (frame < channel->mNumScalingKeys - 1) {
                    if (time < channel->mScalingKeys[frame + 1].mTime)
                        break;
                    frame++;
                }

                presentScaling = channel->mScalingKeys[frame].mValue;
                std::get<2>(m_LastPositions[a]) = frame;
            }

            auto boneindex = boneIndexes.find(channel->mNodeName.data)->second;
            auto &local = m_LocalTransformations[boneindex];
            local.position = { presentPosition.x, presentPosition.y, presentPosition.z };
            local.scale = { presentScaling.x, presentScaling.y, presentScaling.z };
            local.quaternion = { presentRotation.x, presentRotation.y, presentRotation.z, presentRotation.w };
            //local.mat = glm::mat4();
        
            aiMatrix4x4& mat = m_Transformations[a];
            mat = aiMatrix4x4(presentRotation.GetMatrix());
            mat.a1 *= presentScaling.x; mat.b1 *= presentScaling.x; mat.c1 *= presentScaling.x;
            mat.a2 *= presentScaling.y; mat.b2 *= presentScaling.y; mat.c2 *= presentScaling.y;
            mat.a3 *= presentScaling.z; mat.b3 *= presentScaling.z; mat.c3 *= presentScaling.z;
            mat.a4 = presentPosition.x; mat.b4 = presentPosition.y; mat.c4 = presentPosition.z;
            //mat.Transpose();
            local.mat = *(glm::mat4*)&mat;
        }

        m_LastTime = time;
    }
};

class DirectAnimationComponent
    : public TemplateStandardComponent<DirectAnimationComponentEntry, SubSystemId::DirectAnimation>
    //, public Core::Scripts::Component::ComponentEntryWrap<DirectAnimationComponent>
{
public:
    static constexpr char *Name = "DirectAnimation";
    static constexpr bool PublishID = true;

    DirectAnimationComponent(SubsystemManager *Owner);
    virtual ~DirectAnimationComponent();
    virtual bool Initialize() override;
    virtual bool Finalize() override;
    virtual void Step(const Core::MoveConfig &conf) override;
    virtual bool Load(ComponentReader &reader, Entity parent, Entity owner) override;
    virtual bool Create(Entity owner) override;

//	static_assert((sizeof(MeshEntry) % 16) == 0, "Invalid MeshEntry size!");
//	static_assert(std::is_pod<MeshEntry>::value, "ScriptEntry must be pod!");

    using MeshEntry = DirectAnimationComponentEntry;

    static void RegisterScriptApi(ApiInitializer &root);
private:
    template<class T> using Array = Space::Container::StaticVector<T, MoonGlare::Configuration::Storage::ComponentBuffer>;
    
    TransformComponent *m_TransformComponent;

//	::Graphic::Shader *m_ShadowShader = nullptr;
//	::Graphic::Shader *m_GeometryShader = nullptr;

    void ReleaseElement(size_t Index);
};

} //namespace Component 
} //namespace MoonGlare 
