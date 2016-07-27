/*
	Generated by cppsrc.sh
	On 2015-01-19 22:37:49,48
	by Paweu
*/

#pragma once
#ifndef LightSource_H
#define LightSource_H

namespace MoonGlare {
namespace Core {
namespace Objects {

class iLightSource : public cRootClass {
	SPACERTTI_DECLARE_ABSTRACT_CLASS(iLightSource, cRootClass);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
	//DECLARE_SCRIPT_HANDLERS_ROOT(ObjectScriptEvents);
public:
 	iLightSource(Object *Owner, Graphic::Light::LightBase *Base);
 	virtual ~iLightSource();

	static iLightSourcePtr LoadLightSource(Object *Owner, xml_node node);

	virtual void Update(); ///Perform PreRender updates

	virtual bool LoadMeta(const xml_node node);

	virtual bool Initialize();
	virtual bool Finalize();

	using LightType = Graphic::Light::LightType;
	using LightBase = Graphic::Light::LightBase;
	LightType GetType() const { return m_Base->GetType(); }
	LightBase* GetRawPointer() { return m_Base; }

	DefineREADAccesPTR(Owner, Object);

	void SetAmbientIntensity(float v) { m_Base->AmbientIntensity = v; }
	float GetAmbientIntensity() const { return m_Base->AmbientIntensity; }
	void SetDiffuseIntensity(float v) { m_Base->DiffuseIntensity = v; }
	float GetDiffuseIntensity() const { return m_Base->DiffuseIntensity ; }
	void SetColor(const math::vec3 &c) { m_Base->Color = c; }
	const math::vec3& GetColor() const { return m_Base->Color; }
	void SetCastShadows(bool v) { m_Base->CastShadows = v; }
	bool GetCastShadows() const { return m_Base->CastShadows; }

	void Precalculate() { m_Base->Precalculate(); }

	static void RegisterScriptApi(ApiInitializer &api);
private:
	Object *m_Owner = 0;
	Graphic::Light::LightBase *m_Base = 0;
};

} //namespace Objects 
} //namespace Core 
} //namespace MoonGlare 

#endif
