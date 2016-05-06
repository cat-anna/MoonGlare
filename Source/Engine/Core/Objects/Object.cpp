#include <pch.h>
#include <MoonGlare.h>

namespace Core {
namespace Objects {

SPACERTTI_IMPLEMENT_STATIC_CLASS(Object);
IMPLEMENT_SCRIPT_EVENT_VECTOR(ObjectScriptEvents);
RegisterApiDerivedClass(Object, &Object::RegisterScriptApi);

Object::Object():
		BaseClass(),
		m_Flags(),
		m_Visible(true),
		m_Scene(nullptr),
		m_EventProxy(),
		m_LightSource(),
		m_ScriptHandlers(),
		m_PatternName("{?}"),
		m_Mass(1.0f),
		m_Scale(1.0f),
		m_EffectiveScale(1.0f),
		m_OwnerRegister(nullptr),
		m_BodyAngularFactor(1, 1, 1),
		m_PositionTransform(Physics::Quaternion(0,0,0)) {
	m_EventProxy.set(new EventProxy<ThisClass, &ThisClass::InvokeOnTimer>(this));
}

Object::~Object() {
	SetScriptHandlers(new ScriptEventClass());
	m_CollisionMask.Set(Physics::BodyClass::Object);
	m_LightSource.reset();
	m_Body.reset();
}

//---------------------------------------------------------------------------------------

bool Object::Initialize(){
	if (m_LightSource) 
		m_LightSource->Initialize();
	if (m_MoveController) 
		m_MoveController->Initialize();

	if (m_Model) {
		SetShape(m_Model->ConstructShape(GetScale()));
		SetPhysicalProperties(m_Model->GetPhysicalProperties());
	}
	InvokeOnInitialize();
	return true;
}

bool Object::Finalize() {
	InvokeOnFinalize();
	if (m_MoveController) 
		m_MoveController->Finalize();
	if (m_LightSource) 
		m_LightSource->Finalize();
	return true;
}

void Object::RegisterScriptApi(ApiInitializer &api) {
	using MoveControllerTweek = Utils::Template::SmartPointerTweeks<ThisClass, iMoveControllerPtr>;

	struct Helper {
		math::vec3 getpos() {
			return convert(((Object*)this)->GetPosition());
		}
		math::vec3 getlookdir() {
			return convert(((Object*)this)->GetLookDirection());
		}
	};

	api
	.deriveClass<ThisClass, BaseClass>("cObject")
		.addFunction("DropDead", &ThisClass::DropDead)
		.addFunction("IsDead", &ThisClass::IsDead)

		.addFunction("GetHandle", &ThisClass::GetSelfHandle)

		.addFunction("GetMoveController", MoveControllerTweek::Get<&ThisClass::m_MoveController>())
		.addFunction("SetMoveController", MoveControllerTweek::Set<&ThisClass::SetMoveController>())

		.addFunction("InvokeOnDropDead", &ThisClass::InvokeOnDropDead)
		.addFunction("InvokeOnTimer", &ThisClass::InvokeOnTimer)
		.addFunction("InvokeOnInitialize", &ThisClass::InvokeOnInitialize)
		.addFunction("InvokeOnFinalize", &ThisClass::InvokeOnFinalize)
		.addFunction("InvokeEventA", &ThisClass::InvokeOnUserEventA)
		.addFunction("InvokeEventB", &ThisClass::InvokeOnUserEventB)
		.addFunction("InvokeEventC", &ThisClass::InvokeOnUserEventC)
		.addFunction("InvokeEventD", &ThisClass::InvokeOnUserEventD)
		.addFunction("SetEventFunction", &ThisClass::SetEventFunction)
		
		.addFunction("SetTimer", &ThisClass::SetTimer)
		.addFunction("KillTimer", &ThisClass::KillTimer)

		.addFunction("GetLightSource", &ThisClass::GetLightSource)
		//.addFunction("GetMotion", Utils::Template::PointerFromRef<ThisClass, Scene::ModelState>::get<&ThisClass::GetMotionState>())
		
		.addFunction("SetLinearVelocityXYZ", Utils::Template::DynamicArgumentConvert<ThisClass, Physics::vec3, &ThisClass::SetLinearVelocity, float, float, float>::get())
		.addFunction("SetPositionXYZ", Utils::Template::DynamicArgumentConvert<ThisClass, Physics::vec3, &ThisClass::SetPosition, float, float, float>::get())
		.addFunction("SetLookDirectionXYZ", Utils::Template::DynamicArgumentConvert<ThisClass, Physics::vec3, &ThisClass::SetLookDirection, float, float, float>::get())
		.addFunction("SetRotationQuaternion", Utils::Template::DynamicArgumentConvert<ThisClass, Physics::Quaternion, &ThisClass::SetQuaterion, float, float, float, float>::get())
		//.addProperty("Position", &ThisClass::GetPosition, &ThisClass::SetPosition)
		.addFunction("UpdateMotionState", &ThisClass::UpdateMotionState)
		.addFunction("GetPosition", (math::vec3(ThisClass::*)())&Helper::getpos)
		.addFunction("GetLookDirection", (math::vec3(ThisClass::*)())&Helper::getlookdir)
		.endClass();
}

//---------------------------------------------------------------------------------------

void Object::Describe() const {
	char buff[1024] = {};
	auto &pos = GetPosition();
	sprintf(buff, "%s@0x%p [x:%f;y:%f;z:%f]", GetDynamicTypeInfo()->Name, this, 
			pos.x(), pos.y(), pos.z());
	AddLog(Hint, buff << " [" << Info() << "]");
}

void Object::InternalInfo(std::ostringstream &buff) const {
	BaseClass::InternalInfo(buff);
	if(buff.tellp() > 0) buff << " ";
	buff << "Name:'" << GetName() << "'";
}

//---------------------------------------------------------------------------------------

void Object::DropDead(){
	if(IsDead()) return;
	if (InvokeOnDropDead() == 0) {
		m_Flags |= Flags::Dead;
		GetScene()->ObjectDied(GetSelfHandle());
	}
}

//---------------------------------------------------------------------------------------

bool Object::LoadPattern(const xml_node node) {
	auto *pname = node.attribute(xmlAttr_Name).as_string(0);
	if (pname){
		SetPatternName(pname);
		SetName(GetPatternName());
	}
	xml_node light = node.child("LightSource");
	if (light) {
		m_LightSource = iLightSource::LoadLightSource(this, light);
	}
	m_ScriptHandlers->LoadFromXML(node);

	m_Scale = m_EffectiveScale = node.child("Scale").attribute("Value").as_float(1);
	m_Mass = node.child("Mass").attribute("Value").as_float(0);

	if (m_OwnerRegister) {
		auto ParentH = m_OwnerRegister->GetParentHandle(GetSelfHandle());
		auto Parent = m_OwnerRegister->Get(ParentH);
		if (Parent)
			m_EffectiveScale *= Parent->GetEffectiveScale();
	}

	xml_node Model = node.child("Model");
	if (Model) {
		const char *name = Model.attribute(xmlAttr_Name).as_string(0);
		if (!name)
			AddLogf(Error, "Predef object '%s' has defined model without name!", GetName().c_str());
		else {
			SetModel(GetDataMgr()->GetModel(name));
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
	} else {
		GetCollisionMask().Set(Physics::BodyClass::StaticObject);
		GetCollisionMask().Set(Physics::GroupMask::StaticObject);
	}

	XML::Vector::Read(node, "BodyAngularFactor", m_BodyAngularFactor, Physics::vec3(1, 1, 1));

	return LoadDynamicState(node);
}

bool Object::LoadDynamicState(const xml_node node) {
	auto n = node.attribute(xmlAttr_Name).as_string(0);
	if (n) {
		SetName(n);
	}
	math::vec3 pos;
	math::vec4 rot;
	XML::ReadVector(node, "Position", pos, math::vec3(0));
	XML::Vector::Read(node, "Rotation", rot, math::vec4(0, 0, 0, 1));
	SetPosition(convert(pos), Physics::Quaternion(rot[0], rot[1], rot[2], rot[3]));
	return true;
}

//---------------------------------------------------------------------------------------

int Object::InvokeOnDropDead(){
	if (IsDead()) 
		return 0;
	SCRIPT_INVOKE(OnDropDead);
	return 0;
}

int Object::InvokeOnInitialize() { SCRIPT_INVOKE(OnInitialize); }
int Object::InvokeOnFinalize() { SCRIPT_INVOKE(OnFinalize); }
int Object::InvokeOnTimer(int TimerID) { SCRIPT_INVOKE(OnTimer, TimerID); }
int Object::InvokeOnUserEventA(int param) { SCRIPT_INVOKE(OnUserEventA, param); }
int Object::InvokeOnUserEventB(int param) { SCRIPT_INVOKE(OnUserEventB, param); }
int Object::InvokeOnUserEventC(int param) { SCRIPT_INVOKE(OnUserEventC, param); }
int Object::InvokeOnUserEventD(int param) { SCRIPT_INVOKE(OnUserEventD, param); }

//---------------------------------------------------------------------------------------

void Object::SetOwnerScene(GameScene *NewOwner) {
	m_Scene = NewOwner;
	if (HaveBody()) {
		if (NewOwner)
			GetBody()->SetWorldOwner(&NewOwner->GetPhysicsEngine());
		else
			m_Body.reset();
	}
}

//---------------------------------------------------------------------------------------

void Object::ReleaseBody() {
	m_Body->SetWorldOwner(nullptr);
	m_Body.reset();
}

void Object::SetShape(Physics::SharedShape ss) {
	if (m_Body) 
		m_Body.reset();
	if (!ss) 
		return;
	m_Body = Physics::Body::Create(this, ss);
	m_Body->SetCollisionMask(m_CollisionMask);
	m_Body->SetAngularFactor(m_BodyAngularFactor);
	if (GetScene())
		m_Body->SetWorldOwner(&GetScene()->GetPhysicsEngine());
}

void Object::SetMass(float Mass) {
	m_Mass = Mass;
	if (m_Body)
		m_Body->SetMass(Mass);
}

void Object::SetPhysicalProperties(const Physics::PhysicalProperties *props) {
	if (props && m_Body)
		GetBody()->SetPhysicalProperties(*props);
}

void Object::SetMoveController(MoveControllers::iMoveController *ptr) {
	m_MoveController.reset(ptr);
	m_MoveController->SetOwner(this);
}

void Object::SetModel(::DataClasses::ModelPtr Model) {
	m_Model.swap(Model);
	if (m_Model)
		m_Model->Initialize();
}

int Object::SetTimer(float secs, int tid, bool cyclic) { return GetScene()->SetProxyTimer(GetEventProxy(), secs, tid, cyclic); }
void Object::KillTimer(int tid) { return GetScene()->KillProxyTimer(GetEventProxy(), tid); }

} //namespace Objects
} //namespace Core
