#ifndef OBJECT_H
#define OBJECT_H

namespace MoonGlare {
namespace Core {
namespace Objects {

class Object /*final*/ : public NamedObject {
	SPACERTTI_DECLARE_STATIC_CLASS(Object, NamedObject);
	DECLARE_EVENT_HOLDER();
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
	DISABLE_COPY();
public:
	Object();
	virtual ~Object();
	virtual bool Initialize();
	virtual bool Finalize();

	bool LoadPattern(const xml_node node);
	bool LoadDynamicState(const xml_node node);
	
	bool HaveBody() const { return static_cast<bool>(m_Body); }
	Physics::Body* GetBody() const { assert(m_Body); return m_Body.get(); }

	const Physics::vec3& GetPosition() const { return m_PositionTransform.getOrigin(); }
	Physics::Quaternion GetQuaternion() const { return m_PositionTransform.getRotation(); }
	const Physics::vec3& GetLinearVelocity(const Physics::vec3 &sp) { return GetBody()->GetLinearVelocity(); }
	
	void SetPosition(const Physics::vec3 &pos, const Physics::Quaternion &q) { m_PositionTransform = Physics::Transform(q, pos); }
	void SetPosition(const Physics::vec3 &pos) { m_PositionTransform.setOrigin(pos); }
	void SetQuaterion(const Physics::Quaternion &q) { m_PositionTransform.setRotation(q); }
	void SetLinearVelocity(const Physics::vec3 &sp) { GetBody()->SetLinearVelocity(sp); }
	
	void UpdateMotionState() { if(HaveBody()) GetBody()->UpdateMotionState(); }
	DefineRefGetterAll(CollisionMask, Physics::CollisionMask)

	void SetShape(Physics::SharedShape ss);
	void SetMass(float Mass);
	float GetMass() const { return m_Mass; }
	void SetPhysicalProperties(const Physics::PhysicalProperties *prop);

	DefineREADAccesPTR(Scene, Core::GameScene);
	virtual void SetOwnerScene(GameScene *Scene);
	DefineREADAcces(PatternName, string);
	iLightSource* GetLightSource() { return m_LightSource.get(); }
	using BaseClass::SetName;

	DefineDirectSetGet(SelfHandle, Handle);
	DefineDirectSetGet(OwnerRegister, ObjectRegister*);
	DefineRefSetGet(PositionTransform, Physics::Transform);

	void Describe() const;
	static void RegisterScriptApi(ApiInitializer &api);
protected:
	Physics::Transform m_PositionTransform;
	ObjectRegister *m_OwnerRegister;
	Handle m_SelfHandle;
	iLightSourcePtr m_LightSource;

	Physics::CollisionMask m_CollisionMask;
	Physics::BodyPtr m_Body;
	float m_Mass;
	Physics::vec3 m_BodyAngularFactor;// temporary solution

	virtual void InternalInfo(std::ostringstream &buff) const;
private:
	DefineWRITEAcces(PatternName, string);
	Core::GameScene *m_Scene;
	string m_PatternName;

	void ReleaseBody();
};

} //namespace Objects
} //namespace Core
} //namespace MoonGlare 

#endif // OBJECT_H
