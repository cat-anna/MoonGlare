/*
 * ciMap.h
 *
 *  Created on: 09-12-2013
 *      Author: Paweu
 */

#ifndef CIMAP_H_
#define CIMAP_H_

namespace DataClasses {
namespace Maps {

DECLARE_SCRIPT_EVENT_VECTOR(MapScriptEvents, iScriptEvents,
	SCRIPT_EVENT_ADD((AfterLoad)
					 (OnInitialize)(OnFinalize)),
	SCRIPT_EVENT_REMOVE());

class iMap : public DataClass {
	GABI_DECLARE_ABSTRACT_CLASS(iMap, DataClass);
	DECLARE_SCRIPT_HANDLERS_ROOT(MapScriptEvents);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
	DISABLE_COPY();
public:
	iMap(const string &Name);
	virtual ~iMap();

	virtual bool LoadMeta() = 0;
	virtual bool LoadMapObjects(::Core::Objects::ObjectRegister& where); 
	virtual ::Core::Objects::StaticObject* LoadMapObject() = 0;

	virtual bool Initialize() override;
	virtual bool Finalize() override;
	virtual void SetOwnerScene(::Core::Scene::GameScene* Scene);
	
	iPathSharedPtr GetPath(const string& Name) const { return m_PathRegister->GetPath(Name); }

	DefineDirectGetterAll(OwnerScene, ::Core::Scene::GameScene*);
	virtual DataPath GetResourceType() const override final { return DataPath::Maps; }
	
	Graphic::Environment* GetEnvironment() { return &m_Environment; }

	static void RegisterScriptApi(::ApiInitializer &api);
protected:
	virtual bool LoadEnvironment(const xml_node SrcNode);
	bool LoadEvents(const xml_node RootNode);

	xml_node GetPrimaryXML();

	virtual bool DoInitialize() override;
	virtual bool DoFinalize() override;

	virtual int InvokeOnInitialize();
	virtual int InvokeOnFinalize();
	virtual int InvokeAfterLoad();
private:
	Graphic::Environment m_Environment;
	::Core::Scene::GameScene* m_OwnerScene;
	std::unique_ptr<Paths::PathRegister> m_PathRegister;
	FileSystem::XMLFile m_MapMeta;
};

inline void iMapDeleter::operator()(iMap* m) {
	m->Finalize();
	delete m;
}

} // namespace Maps
} // namespace DataClasses

#endif // CIMAP_H_ 
