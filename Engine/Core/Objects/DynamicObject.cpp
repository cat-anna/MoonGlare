#include <pch.h>
#include <MoonGlare.h>

namespace Core {
namespace Objects {

GABI_IMPLEMENT_STATIC_CLASS(DynamicObject);
IMPLEMENT_SCRIPT_EVENT_VECTOR(DynamicObjectScriptEvents);
RegisterApiDerivedClass(DynamicObject, &DynamicObject::RegisterScriptApi);
ObjectClassRegister::Register<DynamicObject> DynamicObjectReg("Object");

DynamicObject::DynamicObject(::Core::GameScene *Scene):
		BaseClass(Scene) {
	SetScriptHandlers(new ScriptEventClass());
	m_CollisionMask.Set(Physics::BodyClass::Object);
}

DynamicObject::~DynamicObject(){
}

//---------------------------------------------------------------------------------------

void DynamicObject::RegisterScriptApi(ApiInitializer &api) {
	using MoveControllerTweek = Utils::Template::SmartPointerTweeks<ThisClass, iMoveControllerPtr>;
	
	api
	.deriveClass<ThisClass, BaseClass>("cDynamicObject")
		.addFunction("GetMoveController", MoveControllerTweek::Get<&ThisClass::m_MoveController>())
		.addFunction("SetMoveController", MoveControllerTweek::Set<&ThisClass::SetMoveController>())

		//.addFunction("InvokeOnCollision", &cObject::InvokeOnCollision)
		//.addFunction("InvokeOnHit", &cObject::InvokeOnHit)
		//.addFunction("InvokeOnCreate", &cObject::InvokeOnCreate)

		//.addFunction("DoHit", &cObject::DoHit)
		//.Member("Size", &cObject::FSize)
		//.addFunction("Stats", &cObject::GetStats)
	.endClass();
}

//---------------------------------------------------------------------------------------

int DynamicObject::InvokeOnDropDead(){
	if (IsDead()) return 0;
	SCRIPT_INVOKE(OnDropDead);
}

int DynamicObject::InvokeOnTimer(int TimerID) { SCRIPT_INVOKE(OnTimer, TimerID); }
int DynamicObject::InvokeOnUserEventA(int param) { SCRIPT_INVOKE(OnUserEventA, param); }
int DynamicObject::InvokeOnUserEventB(int param) { SCRIPT_INVOKE(OnUserEventB, param); }
int DynamicObject::InvokeOnUserEventC(int param) { SCRIPT_INVOKE(OnUserEventC, param); }
int DynamicObject::InvokeOnUserEventD(int param) { SCRIPT_INVOKE(OnUserEventD, param); }

//---------------------------------------------------------------------------------------

bool DynamicObject::Initialize() {
	BaseClass::Initialize();
	if (m_MoveController) m_MoveController->Initialize();
	return true;
}

bool DynamicObject::Finalize() {
	BaseClass::Finalize();
	if (m_MoveController) m_MoveController->Finalize();
	return true;
}

//---------------------------------------------------------------------------------------

void DynamicObject::DoMove(const MoveConfig& conf) {
	BaseClass::DoMove(conf);
	if(m_MoveController) m_MoveController->DoMove(conf);
}

void DynamicObject::PreRender(const PreRenderConfig& conf) {
	BaseClass::PreRender(conf);
}

//---------------------------------------------------------------------------------------

bool DynamicObject::LoadPattern(const xml_node node) {
	BaseClass::LoadPattern(node);

	m_Scale = node.child("Scale").attribute("Value").as_float(1);
	m_Mass = node.child("Mass").attribute("Value").as_float(1);

	if (m_Mass == 0.0f) {
		GetCollisionMask().Set(Physics::BodyClass::StaticObject);
		GetCollisionMask().Set(Physics::GroupMask::StaticObject);
	}

	xml_node Model = node.child("Model");
	if (Model && GetScene()) {
		const char *name = Model.attribute(xmlAttr_Name).as_string(0);
		if (!name)
			AddLogf(Error, "Predef object '%s' has defined model without name!", GetName().c_str());
		else {
			m_ModelInstance = GetScene()->GetInstanceManager().CreateInstance(name);
		}
	}

	xml_node mc = node.child("MoveController");
	if (mc) {
		m_MoveController.reset(MoveControllers::iMoveController::CreateFromXML(mc, this));
	}

	xml_node Collision = node.child("Collision");
	if (Collision) {
		Physics::GroupMaskEnum::LoadFromXML(Collision, m_CollisionMask);
		Physics::BodyClassEnum::LoadFromXML(Collision, m_CollisionMask);
	}

	return true;
#if 0
		if (Flags.attribute("Destroyable").as_bool()) m_Flags |= SPRITE_FLAG_DESTROYABLE;
		if (Flags.attribute("Collision").as_bool()) m_Flags |= SPRITE_FLAG_COLLISION;
		if (Flags.attribute("Visible").as_bool()) m_Flags |= SPRITE_FLAG_VISIBLE;
	}
	//if(!(Node.child("DeltaPos") >> m_Position)) m_Position = math::vec3(0.0f);
	//m_ScriptHandlers->LoadFromXML(Node);
	//m_Stats.LoadFromXML(Node);
	return true;
#endif // 0
}

bool DynamicObject::LoadDynamicState(const xml_node node) {
	BaseClass::LoadDynamicState(node);
	math::vec3 pos, rot;
	XML::ReadVector(node, "Position", pos, math::vec3(0));
	XML::ReadVector(node, "Rotation", rot, math::vec3(0));
	SetPosition(convert(pos), Physics::Quaternion(rot[0], rot[1], rot[2]));
	return true;
}

//---------------------------------------------------------------------------------------

void DynamicObject::SetMoveController(MoveControllers::iMoveController *ptr){
	m_MoveController.reset(ptr);
	m_MoveController->SetOwner(this);
}

#if 0
bool Object::DoHit(Object *Attacker, Core::cDamages *Damage){
//	if(m_Flags & SPRITE_FLAG_DEAD) return false;
//	if(!(m_Flags & SPRITE_FLAG_DESTROYABLE)) return false;
//	return Core::GetEngine()->GetGameEngine()->PerformHit(Attacker, this, Damage);
	LOG_NOT_IMPLEMENTED
	return false;
}

bool Object::SaveToXML(xml_node Node, bool OnlyPosition){
	bool succ = true;
//	m_Position.SaveToXML(Node, 0);
	Node.append_attribute("Name") = GetName().c_str();
	if(OnlyPosition) return true;

	//succ &= m_Stats.SaveToXML(Node);
	if(m_MoveController)
		succ &= m_MoveController->SaveToXML(Node.append_child("MoveController"));
	
	return succ;
}

bool Object::LoadFromXML(xml_node Node){
	//m_Stats.LoadFromXML(Node);
	math::vec3 DeltaPos(0.0f);
	//if(Node >> DeltaPos) m_Position += DeltaPos;
	string name = Node.attribute("Name").as_string("");
	if(!name.empty()) m_Name = name;
	xml_node mc = Node.child("MoveController");
	if(mc) {
		if(m_MoveController) delete m_MoveController;
		m_MoveController = iMoveController::CreateFromXML(mc, this);
	}
	return true;
}

bool Object::SaveToPattern(xml_node Node){
	/*Node.append_attribute("Width") = m_Size.x;
	Node.append_attribute("Height") = m_Size.y;*/
	xml_node Flags = Node.append_child("Flags");

	//Node.append_child("Rotation") << m_Rotation;
	//Node.append_child("Scale") << m_Scale;

	//m_Stats.SaveToXML(Node);
	//m_ScriptHandlers->SaveToXML(Node);
	if(m_MoveController)
		m_MoveController->SaveToXML(Node.append_child("MoveController"));
	return true;
}

#endif

} //namespace Objects
} //namespace Core
