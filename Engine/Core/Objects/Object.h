#ifndef OBJECT_H
#define OBJECT_H

namespace Core {
namespace Objects {

DECLARE_SCRIPT_EVENT_VECTOR(ObjectScriptEvents, iScriptEvents,
		SCRIPT_EVENT_ADD(
			(OnDropDead)(OnTimer)
			(OnInitialize)(OnFinalize)
			(OnUserEventA)(OnUserEventB)(OnUserEventC)(OnUserEventD)
		),
		SCRIPT_EVENT_REMOVE());

class Object : public NamedObject {
	GABI_DECLARE_ABSTRACT_CLASS(Object, NamedObject);
	DECLARE_SCRIPT_HANDLERS_ROOT(ObjectScriptEvents);
	DECLARE_EVENT_HOLDER();
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	virtual ~Object();
	virtual bool Initialize();
	virtual bool Finalize();

	virtual void DoMove(const MoveConfig &conf);
	virtual void PreRender(const PreRenderConfig& conf);

	virtual void DropDead();

	virtual bool LoadPattern(const xml_node node);
	virtual bool LoadDynamicState(const xml_node node);
	
	struct Flags {
		enum {
			Dead				= 0x0001,
			Initialized			= 0x1000,
			ModelStateChanged	= 0x1000,
		};
	};
	DefineFlagGetter(m_Flags, Flags::Dead, Dead);
	DefineFlagGetter(m_Flags, Flags::Initialized, Initialized);

	bool HaveBody() const { return static_cast<bool>(m_Body); }
	Physics::Body* GetBody() const { assert(m_Body); return m_Body.get(); }

	const Physics::vec3& GetPosition() const { return GetMotionState().m_graphicsWorldTrans.getOrigin(); }
	Physics::Quaternion GetQuaternion() const { return GetMotionState().m_graphicsWorldTrans.getRotation(); }
	const Physics::vec3& GetLinearVelocity(const Physics::vec3 &sp) { return GetBody()->GetLinearVelocity(); }
	
	void SetPosition(const Physics::vec3 &pos, const Physics::Quaternion &q) { GetMotionState().m_graphicsWorldTrans = Physics::Transform(q, pos); }
	void SetPosition(const Physics::vec3 &pos) { GetMotionState().m_graphicsWorldTrans.setOrigin(pos); }
	void SetQuaterion(const Physics::Quaternion &q) { GetMotionState().m_graphicsWorldTrans.setRotation(q); }
	void SetLinearVelocity(const Physics::vec3 &sp) { GetBody()->SetLinearVelocity(sp); }
	
	void UpdateMotionState() { GetBody()->UpdateMotionState(); }
	DefineSetGetByRef(LookDirection, Physics::vec3);
	DefineSetGetByRef(MotionState, Physics::DefaultMotionState);
	DefineRefGetterAll(CollisionMask, Physics::CollisionMask)

	void SetShape(Physics::SharedShape ss);
	void SetMass(float Mass);
	float GetMass() const { return m_Mass; }
	void SetPhysicalProperties(const Physics::PhysicalProperties *prop);

	virtual int InvokeOnDropDead();
	virtual int InvokeOnInitialize();
	virtual int InvokeOnFinalize();
	virtual int InvokeOnTimer(int TimerID);
	virtual int InvokeOnUserEventA(int param);
	virtual int InvokeOnUserEventB(int param);
	virtual int InvokeOnUserEventC(int param);
	virtual int InvokeOnUserEventD(int param);

	int SetTimer(float secs, int tid, bool cyclic) { return GetScene()->SetProxyTimer(GetEventProxy(), secs, tid, cyclic); }
	void KillTimer(int tid) { return GetScene()->KillProxyTimer(GetEventProxy(), tid); }

	DefineREADAccesPTR(Scene, ::Core::GameScene);
	virtual void SetOwnerScene(GameScene *Scene);
	DefineREADAcces(PatternName, string);
	iLightSource* GetLightSource() { return m_LightSource.get(); }
	float GetScale() const { return m_Scale; }
	using BaseClass::SetName;

	void Describe() const;
	static void RegisterScriptApi(ApiInitializer &api);
protected:
	iLightSourcePtr m_LightSource;
	Scene::ModelInstancePtr m_ModelInstance;
	unsigned m_Flags;

	Physics::CollisionMask m_CollisionMask;
	Physics::BodyPtr m_Body;
	Physics::vec3 m_LookDirection; /** Look direction of object */
	Physics::DefaultMotionState m_MotionState;
	float m_Mass, m_Scale;

	Object(::Core::GameScene *Scene);
	virtual void OnBodyConstruction();
	virtual void InternalInfo(std::ostringstream &buff) const;
private:
	DefineWRITEAcces(PatternName, string);
	string m_PatternName;
	::Core::GameScene *m_Scene;

	void ReleaseBody();
};

} //namespace Objects
} //namespace Core

#endif // OBJECT_H
