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

class Object /*final*/ : public NamedObject {
	GABI_DECLARE_STATIC_CLASS(Object, NamedObject);
	DECLARE_SCRIPT_HANDLERS_ROOT(ObjectScriptEvents);
	DECLARE_EVENT_HOLDER();
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
	DISABLE_COPY();
public:
	Object();
	virtual ~Object();
	virtual bool Initialize();
	virtual bool Finalize();

	void DoMove(const MoveConfig &conf);

	void DropDead();

	bool LoadPattern(const xml_node node);
	bool LoadDynamicState(const xml_node node);
	
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

	const Physics::vec3& GetPosition() const { return m_PositionTransform.getOrigin(); }
	Physics::Quaternion GetQuaternion() const { return m_PositionTransform.getRotation(); }
	const Physics::vec3& GetLinearVelocity(const Physics::vec3 &sp) { return GetBody()->GetLinearVelocity(); }
	
	void SetPosition(const Physics::vec3 &pos, const Physics::Quaternion &q) { m_PositionTransform = Physics::Transform(q, pos); }
	void SetPosition(const Physics::vec3 &pos) { m_PositionTransform.setOrigin(pos); }
	void SetQuaterion(const Physics::Quaternion &q) { m_PositionTransform.setRotation(q); }
	void SetLinearVelocity(const Physics::vec3 &sp) { GetBody()->SetLinearVelocity(sp); }
	
	void UpdateMotionState() { if(HaveBody()) GetBody()->UpdateMotionState(); }
	DefineSetGetByRef(LookDirection, Physics::vec3);
	DefineRefGetterAll(CollisionMask, Physics::CollisionMask)

	void SetShape(Physics::SharedShape ss);
	void SetMass(float Mass);
	float GetMass() const { return m_Mass; }
	void SetPhysicalProperties(const Physics::PhysicalProperties *prop);

	int InvokeOnDropDead();
	int InvokeOnInitialize();
	int InvokeOnFinalize();
	int InvokeOnTimer(int TimerID);
	int InvokeOnUserEventA(int param);
	int InvokeOnUserEventB(int param);
	int InvokeOnUserEventC(int param);
	int InvokeOnUserEventD(int param);

	int SetTimer(float secs, int tid, bool cyclic);
	void KillTimer(int tid);

	DefineRefGetterAll(MoveController, iMoveControllerPtr);
	DefineREADAccesPTR(Scene, ::Core::GameScene);
	virtual void SetOwnerScene(GameScene *Scene);
	DefineREADAcces(PatternName, string);
	iLightSource* GetLightSource() { return m_LightSource.get(); }
	float GetScale() const { return m_Scale; }
	using BaseClass::SetName;

	DefineDirectSetGet(Visible, bool);

	DefineDirectSetGet(SelfHandle, Handle);
	DefineDirectSetGet(OwnerRegister, ObjectRegister*);
	DefineRefSetGet(PositionTransform, Physics::Transform);

	void SetModel(::DataClasses::ModelPtr Model);
	::DataClasses::ModelPtr& GetModel() { return m_Model; }

	void Describe() const;
	static void RegisterScriptApi(ApiInitializer &api);
protected:
	Physics::Transform m_PositionTransform;
	ObjectRegister *m_OwnerRegister;
	Handle m_SelfHandle;
	iLightSourcePtr m_LightSource;
	unsigned m_Flags;
	bool m_Visible;
	::DataClasses::ModelPtr m_Model;

	Physics::CollisionMask m_CollisionMask;
	Physics::BodyPtr m_Body;
	Physics::vec3 m_LookDirection; /** Look direction of object */
	float m_Mass, m_Scale;
	Physics::vec3 m_BodyAngularFactor;// temporary solution

	virtual void InternalInfo(std::ostringstream &buff) const;
	void SetMoveController(MoveControllers::iMoveController *ptr);
private:
	DefineWRITEAcces(PatternName, string);
	iMoveControllerPtr m_MoveController;
	::Core::GameScene *m_Scene;
	string m_PatternName;

	void ReleaseBody();
};

} //namespace Objects
} //namespace Core

#endif // OBJECT_H
