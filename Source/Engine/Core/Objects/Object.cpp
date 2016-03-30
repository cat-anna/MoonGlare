#include <pch.h>
#include <MoonGlare.h>

namespace Core {
namespace Objects {

GABI_IMPLEMENT_STATIC_CLASS(Object);
IMPLEMENT_SCRIPT_EVENT_VECTOR(ObjectScriptEvents);
RegisterApiDerivedClass(Object, &Object::RegisterScriptApi);

Object::Object(::Core::GameScene *Scene):
		BaseClass(),
		m_Flags(),
		m_ModelInstance(),
		m_Scene(Scene),
		m_EventProxy(),
		m_LightSource(),
		m_ScriptHandlers(),
		m_PatternName("{?}"),
		m_Mass(1.0f),
		m_Scale(1.0f) {
	m_EventProxy.set(new EventProxy<ThisClass, &ThisClass::InvokeOnTimer>(this));
}

Object::~Object() {
	SetScriptHandlers(new ScriptEventClass());
	m_CollisionMask.Set(Physics::BodyClass::Object);
	m_ModelInstance.reset();
	m_LightSource.reset();
	m_Body.reset();
}

//---------------------------------------------------------------------------------------

bool Object::Initialize(){
	if (m_LightSource) m_LightSource->Initialize();
	if (m_ModelInstance) m_ModelInstance->Initialize(this);
	if (m_MoveController) m_MoveController->Initialize();
	InvokeOnInitialize();
	return true;
}

bool Object::Finalize() {
	InvokeOnFinalize();
	if (m_MoveController) m_MoveController->Finalize();
	if (m_LightSource) m_LightSource->Finalize();
	if (m_ModelInstance) m_ModelInstance->Finalize();
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
	.deriveClass<ThisClass, BaseClass>("cObjectBase")
		.addFunction("DropDead", &ThisClass::DropDead)
		.addFunction("IsDead", &ThisClass::IsDead)

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

void Object::DoMove(const MoveConfig &conf) {
	if (m_MoveController) m_MoveController->DoMove(conf);
	if (m_LightSource) m_LightSource->DoMove(conf);
}

void Object::PreRender(const PreRenderConfig& conf) {
	if (m_LightSource) m_LightSource->Update();
	if (m_ModelInstance) m_ModelInstance->Update(this);
}

void Object::DropDead(){
	if(IsDead()) return;
	if (InvokeOnDropDead() == 0) {
		m_Flags |= Flags::Dead;
		GetScene()->ObjectDied(this);
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

	m_Scale = node.child("Scale").attribute("Value").as_float(1);
	m_Mass = node.child("Mass").attribute("Value").as_float(0);

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
}

bool Object::LoadDynamicState(const xml_node node) {
	auto n = node.attribute(xmlAttr_Name).as_string(0);
	if (n) {
		SetName(n);
	}
	math::vec3 pos, rot;
	XML::ReadVector(node, "Position", pos, math::vec3(0));
	XML::ReadVector(node, "Rotation", rot, math::vec3(0));
	SetPosition(convert(pos), Physics::Quaternion(rot[0], rot[1], rot[2]));
	return true;
}

//---------------------------------------------------------------------------------------

int Object::InvokeOnDropDead(){
	if (IsDead()) return 0;
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
}

//---------------------------------------------------------------------------------------

void Object::OnBodyConstruction() {
	m_Body->SetCollisionMask(m_CollisionMask);

}

void Object::ReleaseBody() {
	m_Body->SetWorldOwner(nullptr);
	m_Body.reset();
}

void Object::SetShape(Physics::SharedShape ss) {
	if (m_Body) m_Body.reset();
	if (!ss) return;
	m_Body = Physics::Body::Create(this, ss);
	OnBodyConstruction();
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

} //namespace Objects
} //namespace Core
