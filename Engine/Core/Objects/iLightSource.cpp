/*
	Generated by cppsrc.sh
	On 2015-01-19 22:37:49,48
	by Paweu
*/
#include <pch.h>
#include <MoonGlare.h>

namespace Core {
namespace Objects {

//---------------------------------------------------------------------------------------

class PointLightSource : public iLightSource {
	GABI_DECLARE_CLASS_NOCREATOR(PointLightSource, iLightSource);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	PointLightSource(Object *Owner) : BaseClass(Owner, &m_light) { }
	~PointLightSource() { }

	void Update() override {
		m_light.Position = convert(GetOwner()->GetPosition()) + m_PositionDelta;
	}
	bool LoadMeta(const xml_node node) override { 
		XML::Vector::Read(node, "PositionDelta", m_PositionDelta);
		return m_light.LoadMeta(node); 
	}
	bool Initialize() override { 
		m_light.Initialize();
		m_light.Precalculate();
		return BaseClass::Initialize(); 
	}

	void SetPosition(const math::vec3 &d) { m_PositionDelta = d; }
	const math::vec3& GetPosition() const { return m_PositionDelta; }
	void SetPosition3f(float r, float g, float b) { m_light.Position = math::vec3(r, g, b); }

	Graphic::Light::LightAttenuation* GetAttenuation() { return &m_light.Attenuation; }

	static void RegisterScriptApi(ApiInitializer &api){
		api
		.deriveClass<ThisClass, BaseClass>("cPointLightSource")
			.addData("Position", &ThisClass::m_PositionDelta)
			.addFunction("SetPosition", &ThisClass::SetPosition3f)
			.addFunction("GetAttenuation", &ThisClass::GetAttenuation)
		.endClass();
	}	
protected:
	math::vec3 m_PositionDelta{0, 0, 0};
	Graphic::Light::PointLight m_light;
};

GABI_IMPLEMENT_CLASS_NOCREATOR(PointLightSource);
RegisterApiDerivedClass(PointLightSource, &PointLightSource::RegisterScriptApi);
LightSoureClassRegister::Register<PointLightSource> PointLightSourceReg;

//---------------------------------------------------------------------------------------

class DirectionalLightSource: public iLightSource {
	GABI_DECLARE_CLASS_NOCREATOR(DirectionalLightSource, iLightSource);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	DirectionalLightSource(Object *Owner) : BaseClass(Owner, &m_light) { }
	~DirectionalLightSource() {}

	void Update() override { }

	bool LoadMeta(const xml_node node) override { return m_light.LoadMeta(node); }
	bool Initialize() override { 
		m_light.Initialize();
		m_light.Precalculate();
		return BaseClass::Initialize(); 
	}

	void SetDirection(const math::vec3 &d) { m_light.Direction = d; }
	const math::vec3& GetDirection() const { return m_light.Direction; }
	void SetDirection3f(float r, float g, float b) { m_light.Direction = math::vec3(r, g, b); }

	static void RegisterScriptApi(ApiInitializer &api){
		api
		.deriveClass<ThisClass, BaseClass>("cDirectionalLightSource")
			.addProperty("Direction", &ThisClass::GetDirection, &ThisClass::SetDirection)
			.addFunction("SetDirection", &ThisClass::SetDirection3f)
		.endClass();
	} 
protected:
	Graphic::Light::DirectionalLight m_light;
};

GABI_IMPLEMENT_CLASS_NOCREATOR(DirectionalLightSource);
RegisterApiDerivedClass(DirectionalLightSource, &DirectionalLightSource::RegisterScriptApi);
LightSoureClassRegister::Register<DirectionalLightSource> DirectionalLightSourceReg;

//---------------------------------------------------------------------------------------

class SpotLightSource: public iLightSource {
	GABI_DECLARE_CLASS_NOCREATOR(SpotLightSource, iLightSource);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	SpotLightSource(Object *Owner) : BaseClass(Owner, &m_light) { }
	~SpotLightSource() {}

	void Update() override {
		m_light.Position = convert(GetOwner()->GetPosition()) + m_PositionDelta;
		//m_light.Position[1] += 0.5f;
		m_light.RecalculateMatrices();
	}

	bool LoadMeta(const xml_node node) override { 
		XML::Vector::Read(node, "PositionDelta", m_PositionDelta);
		return m_light.LoadMeta(node); 
	}
	bool Initialize() override { 
		m_light.Initialize();
		m_light.Precalculate();
		return BaseClass::Initialize(); 
	}

	void SetPosition(const math::vec3 &d) { m_PositionDelta = d; }
	const math::vec3& GetPosition() const { return m_PositionDelta; }
	void SetDirection(const math::vec3 &d) { m_light.Direction = d; }
	const math::vec3& GetDirection() const { return m_light.Direction; }
	void SetDirection3f(float r, float g, float b) { m_light.Direction = math::vec3(r, g, b); }
	void SetPosition3f(float r, float g, float b) { m_light.Position = math::vec3(r, g, b); }

	Graphic::Light::LightAttenuation* GetAttenuation() { return &m_light.Attenuation; }

	float GetCutOff() const { return m_light.CutOff; }
	void SetCutOff(float f) { m_light.SetCutOff(f); }

	static void RegisterScriptApi(ApiInitializer &api){
		api
		.deriveClass<ThisClass, BaseClass>("cSpotLightSource")
			.addData("Position", &ThisClass::m_PositionDelta)
			.addProperty("Direction", &ThisClass::GetDirection, &ThisClass::SetDirection)
			.addFunction("GetAttenuation", &ThisClass::GetAttenuation)
			.addFunction("SetDirection", &ThisClass::SetDirection3f)
			.addFunction("SetPosition", &ThisClass::SetPosition3f)
			.addProperty("CutOff", &ThisClass::GetCutOff, &ThisClass::SetCutOff)
		.endClass();
	}
protected:
	math::vec3 m_PositionDelta{0, 0, 0};
	Graphic::Light::SpotLight m_light;
};

GABI_IMPLEMENT_CLASS_NOCREATOR(SpotLightSource);
RegisterApiDerivedClass(SpotLightSource, &SpotLightSource::RegisterScriptApi);
LightSoureClassRegister::Register<SpotLightSource> SpotLightSourceReg;

//---------------------------------------------------------------------------------------

GABI_IMPLEMENT_ABSTRACT_CLASS(iLightSource);
RegisterApiDerivedClass(iLightSource, &iLightSource::RegisterScriptApi);

iLightSource::iLightSource(Object *Owner, Graphic::Light::LightBase *Base):
		BaseClass(),
		m_Owner(Owner),
		m_Base(Base) {
}

iLightSource::~iLightSource() {
}

void iLightSource::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("iLightSource")
		.addProperty("Ambient", &ThisClass::GetAmbientIntensity, &ThisClass::SetAmbientIntensity)
		.addProperty("Diffuse", &ThisClass::GetDiffuseIntensity, &ThisClass::SetDiffuseIntensity)
		.addProperty("Color", &ThisClass::GetColor, &ThisClass::SetColor)
		.addProperty("Type", &ThisClass::GetType)
		.addProperty("CastShadows", &ThisClass::GetCastShadows, &ThisClass::SetCastShadows)

		.addFunction("SetColor",  Utils::Template::DynamicArgumentConvert<ThisClass, math::vec3, &ThisClass::SetColor, float, float, float>::get())
		.addFunction("Precalculate", &ThisClass::Precalculate)
	.endClass()
	.beginClass<Graphic::Light::LightAttenuation>("cLightAttenuation")
		.addData("Constant", &Graphic::Light::LightAttenuation::Constant)
		.addData("Linear", &Graphic::Light::LightAttenuation::Linear)
		.addData("Exp", &Graphic::Light::LightAttenuation::Exp)
		.addData("MinThreshold", &Graphic::Light::LightAttenuation::MinThreshold)
	.endClass()
	;
}
 
//---------------------------------------------------------------------------------------

bool iLightSource::LoadMeta(const xml_node node) {
	return false;
}

iLightSourcePtr iLightSource::LoadLightSource(Object *Owner, xml_node node) {
	if (!Owner) {
		AddLog(Error, "Attempt to create LightSource without Owner!");
		return 0;
	}
	if(!node) return 0;
	const char* Class = node.attribute(xmlAttr_Class).as_string(0);
	if (!Class) {
		AddLog(Error, "LightSource definition without a class name!");
		return 0;
	}
	auto item = LightSoureClassRegister::CreateClass(Class, Owner);
	if (!item->LoadMeta(node)) {
		AddLogf(Error, "An error has occur during loading light source of class '%s' for object '%s'", Class, Owner->GetName().c_str());
		delete item;
		return nullptr;
	}
	return iLightSourcePtr(item);
}

//---------------------------------------------------------------------------------------

bool iLightSource::Initialize() {
	GetOwner()->GetScene()->AddLightSource(this);
	return true; 
}

bool iLightSource::Finalize() { 
	GetOwner()->GetScene()->RemoveLightSource(this);
	return true; 
}

//---------------------------------------------------------------------------------------

void iLightSource::DoMove(const MoveConfig& conf) {
	//nothing here
}

void iLightSource::Update() {
	//nothing here
}

} //namespace Objects 
} //namespace Core 
