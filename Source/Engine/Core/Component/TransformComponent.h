/*
  * Generated by cppsrc.sh
  * On 2015-10-25 21:57:04,09
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef TransformComponent_H
#define TransformComponent_H

namespace MoonGlare {
namespace Core {
namespace Component {

class TransformComponent : public AbstractComponent {
public:
	TransformComponent(ComponentManager *Owner);
	virtual ~TransformComponent();
	virtual bool Initialize() override;
	virtual bool Finalize() override;
	virtual void Step(const MoveConfig &conf) override;
	virtual Handle Load(xml_node node, Entity Owner) override;
	constexpr static ComponentID GetComponentID() { return 2; };


	struct TransformEntry {
		Entity m_Owner;

		math::mat4 m_LocalMatrix;
		math::mat4 m_GlobalMatrix;
		Physics::vec3 m_LocalScale;
		Physics::vec3 m_GlobalScale;

		Physics::Transform m_Transform;
		Physics::Transform m_CenterOfMass;
	};

//	struct BulletMotionStateProxy : public btMotionState {
//		///synchronizes world transform from user to physics
//		virtual void getWorldTransform(btTransform& centerOfMassWorldTrans) const override;
//		///synchronizes world transform from physics to user
//		///Bullet only calls the update of worldtransform for active objects
//		virtual void setWorldTransform(const btTransform& centerOfMassWorldTrans) override;
//	};

//	TransformEntry* GetForEntry();

/*
	btTransform
	
	d3		position
	d4		quaternion
	mat3x3	matrix
	?scale

	//process (position & quaternion) -> matrix  [done by physics engine]
	//static & dynamic <- future optimalisation
	indirect 
*/

protected:
	template<class T> using Array = std::array<T, Configuration::Storage::ComponentBuffer>;
	Array<TransformEntry> m_Array;
//	Array<BulletMotionStateProxy> m_Proxies;
};

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 

#endif
