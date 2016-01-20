/*
 * GameScene.h
 *
 *  Created on: 17-12-2013
 *      Author: Paweu
 */

#ifndef GameScene_H_
#define GameScene_H_

namespace Core {
namespace Scene {

class GameScene : public ciScene {
	GABI_DECLARE_CLASS(GameScene, ciScene)
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
	DISABLE_COPY();
public:
	GameScene();
	virtual ~GameScene();

	using Objecy = ::Core::Objects::Object;

	virtual int InvokeOnTimer(int TimerID) override;
	virtual int InvokeOnEscape() override;
	virtual int InvokeOnBeginScene() override;
	virtual int InvokeOnEndScene() override;
	virtual int InvokeOnInitialize() override;
	virtual int InvokeOnFinalize() override;

	virtual void BeginScene() override;
	virtual void EndScene() override;

	virtual void DoMove(const MoveConfig &conf) override;
	virtual void PreRender(const PreRenderConfig& conf);
	virtual void DoRender(cRenderDevice& Dev) const override;
	virtual void DoRenderMeshes(cRenderDevice &Dev) const override;
	virtual Graphic::Light::LightConfiguration* GetLightConfig();

	ModelInstanceManager& GetInstanceManager() { return *m_ModelIntances; }
	Physics::PhysicEngine& GetPhysicsEngine() { return *m_Physics; }

	void AddLightSource(::Core::Objects::iLightSource *ptr);
	void RemoveLightSource(::Core::Objects::iLightSource *ptr);
	Object* CreateObject(const string& TypeName, const string& Name = "");

	Object* SpawnObject(const string& TypeName, const string& Name, const Physics::vec3& Position);
	Object* SpawnObject_api(const string& TypeName, const string& Name, const math::vec3 &pos);

	Object* GetObjectByName(const string& Name);
	const Objects::ObjectList& GetObjectsByName(const string& Name);
	const Objects::ObjectList& GetObjectsByType(const string& Type);

	void ObjectDied(::Core::Objects::Object *object);

	const DataClasses::MapPtr& GetMapInstance() const { return m_MapData; }

	static void RegisterScriptApi(::ApiInitializer &api);
//old
protected:
	string m_MapName;
	DataClasses::MapPtr m_MapData;
	std::unique_ptr<Objects::ObjectRegister> m_Objects;
	std::unique_ptr<ModelInstanceManager> m_ModelIntances;
	std::list<Object*> m_DeadList;

	Physics::PhysicEnginePtr m_Physics;
	Graphic::Environment *m_Environment;
	Graphic::Light::LightConfiguration m_LightConfiguration;

	virtual bool DoInitialize() override;
	virtual bool DoFinalize() override;
	virtual bool LoadMeta(const xml_node Node) override;
};

} // namespace Scene 
} // namespace Core 

#endif // GameScene_H_ 
