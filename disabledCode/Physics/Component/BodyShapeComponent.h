/*
  * Generated by cppsrc.sh
  * On 2016-08-30 21:19:19,54
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef CollisionComponent_H
#define CollisionComponent_H

#include <libSpace/src/Container/StaticVector.h>

#include <Core/Component/TemplateStandardComponent.h>

namespace MoonGlare {
namespace Physics {
namespace Component {

using namespace ::MoonGlare::Core::Component;
using namespace ::Physics;

enum class ColliderType : uint8_t {
    Unknown,
    Box,
    Sphere,
    ConvexMesh,
    TriangleMesh,
    Capsule,
    Cylinder,
};

struct BodyShapeComponentEntry {
    union FlagsMap {
        struct MapBits_t {
            bool m_Valid : 1; //Entity is not valid or requested to be deleted;
        };
        MapBits_t m_Map;
        uint32_t m_UintValue;

        void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
        void ClearAll() { m_UintValue = 0; }

        static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
    };

    Entity m_OwnerEntity;
    FlagsMap m_Flags;
    BodyComponent *m_BodyComponent;
    BodyShapeComponent *shapeComponent;
    std::unique_ptr<btCollisionShape> m_Shape;
    std::unique_ptr<btTriangleIndexVertexArray> meshInterface;

    bool SetShapeInternal(std::unique_ptr<btCollisionShape> shape);
    void SetShape(btCollisionShape *shape);
    void SetSphere(float Radius);
    void SetBox(const math::vec3 & size);

    void SetTriangleMesh(Renderer::MeshResourceHandle h);
    void SetConvexMesh(Renderer::MeshResourceHandle h);
};                

class BodyShapeComponent
    : public AbstractSubsystem
    , public SubSystemIdWrap<Core::Component::SubSystemId::BodyShape> {
public:                    
    friend struct BodyShapeComponentEntry;

    BodyShapeComponent(Core::Component::SubsystemManager *Owner);
    virtual ~BodyShapeComponent();

    virtual bool Initialize() override;
    virtual bool Finalize() override;

    virtual void Step(const Core::MoveConfig &conf) override;

    virtual bool Load(ComponentReader &reader, Entity parent, Entity owner) override;

    virtual bool PushEntryToLua(Entity e, lua_State *lua, int &luarets);
    bool Create(Entity Owner) override;

    BodyShapeComponentEntry* GetEntry(Entity e) {
        auto index = m_EntityMapper.GetIndex(e);
        if (index == ComponentIndex::Invalid)
            return nullptr;
        return &m_Array[index];
    }

//	virtual bool LoadComponentConfiguration(pugi::xml_node node);
/*
btCylinderShape
btCapsuleShapeX
btCapsuleShapeZ
btConvexHullShape
btBvhTriangleMeshShape
*/
    static MoonGlare::Scripts::ApiInitializer RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root);
protected:
    template<class T> using Array = Space::Container::StaticVector<T, MoonGlare::Configuration::Storage::ComponentBuffer>;

    BodyComponent *m_BodyComponent = nullptr;
    TransformComponent *m_TransformComponent = nullptr;

    Array<BodyShapeComponentEntry> m_Array;

    EntityArrayMapper<> m_EntityMapper;

    bool BuildEntry(Entity Owner, size_t &indexout);

    std::pair<std::unique_ptr<btCollisionShape>, ColliderType> LoadByName(const std::string &name, xml_node node);
    std::pair<std::unique_ptr<btCollisionShape>, ColliderType> LoadShape(xml_node node, Entity Owne);
};

} //namespace Component 
} //namespace Physics 
} //namespace MoonGlare 

#endif