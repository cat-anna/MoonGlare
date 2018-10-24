#pragma once

#include <libSpace/src/Container/StaticVector.h>

#include <Core/Component/TemplateStandardComponent.h>
#include <Foundation/Resources/SkeletalAnimationManager.h>

namespace MoonGlare::Component {
using namespace MoonGlare::Core::Component;

class TransformComponent;

struct DirectAnimationComponentEntry {
    union FlagsMap {
        struct MapBits_t {
            bool m_Valid : 1; //Entity is not valid or requested to be deleted;
            //bool m_AnimValid : 1;
            //bool m_Loop : 1;
            //bool m_Playing : 1;
        };
        MapBits_t m_Map;
        uint32_t m_UintValue;

        void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
        void ClearAll() { m_UintValue = 0; }

        static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
    };

    FlagsMap m_Flags;
    Entity m_Owner;

    uint8_t validBones = 0;
    Entity bones[64];
    Resources::AnimationBlendState blendState;

    void Reset() {
        m_Flags.m_Map.m_Valid = false;
    }
};

class DirectAnimationComponent
    : public TemplateStandardComponent<DirectAnimationComponentEntry, SubSystemId::DirectAnimation>
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
private:
    TransformComponent *m_TransformComponent = nullptr;
    Resources::SkeletalAnimationManager* skeletalAnimationManager = nullptr;
};

} 
