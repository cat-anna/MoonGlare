#include <pch.h>
#include <MoonGlare.h>

namespace MoonGlare {
namespace Core {
namespace Objects {

SPACERTTI_IMPLEMENT_STATIC_CLASS(Object);
RegisterApiDerivedClass(Object, &Object::RegisterScriptApi);

Object::Object():
		BaseClass(),
		m_Scene(nullptr),
		m_EventProxy(),
		m_PatternName("{?}"),
		m_Mass(1.0f),
		m_OwnerRegister(nullptr),
		m_BodyAngularFactor(1, 1, 1),
		m_PositionTransform(Physics::Quaternion(0,0,0)) {
}

Object::~Object() {
	m_CollisionMask.Set(Physics::BodyClass::Object);
	m_Body.reset();
}

//---------------------------------------------------------------------------------------

bool Object::Initialize(){
	return true;
}

bool Object::Finalize() {
	return true;
}

void Object::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cObject")
	.endClass();
}

//---------------------------------------------------------------------------------------

void Object::Describe() const {
	char buff[1024] = {};
	auto &pos = GetPosition();
	sprintf(buff, "%s@0x%p [x:%f;y:%f;z:%f]", GetDynamicTypeInfo()->GetName(), this, 
			pos.x(), pos.y(), pos.z());
	AddLog(Hint, buff << " [" << Info() << "]");
}

void Object::InternalInfo(std::ostringstream &buff) const {
	BaseClass::InternalInfo(buff);
	if(buff.tellp() > 0) buff << " ";
	buff << "Name:'" << GetName() << "'";
}

//---------------------------------------------------------------------------------------

bool Object::LoadPattern(const xml_node node) {
	auto *pname = node.attribute(xmlAttr_Name).as_string(0);
	if (pname){
		SetPatternName(pname);
		SetName(GetPatternName());
	}

	m_Mass = node.child("Mass").attribute("Value").as_float(0);

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

} //namespace Objects
} //namespace Core
} //namespace MoonGlare 
