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

	virtual int InvokeOnTimer(int TimerID) override;
	virtual int InvokeOnEscape() override;
	virtual int InvokeOnBeginScene() override;
	virtual int InvokeOnEndScene() override;
	virtual int InvokeOnInitialize() override;
	virtual int InvokeOnFinalize() override;

	virtual void BeginScene() override;
	virtual void EndScene() override;

	virtual void DoMove(const MoveConfig &conf) override;
	virtual Graphic::Light::LightConfiguration* GetLightConfig();

	Physics::PhysicEngine& GetPhysicsEngine() { return *m_Physics; }

	void AddLightSource(::Core::Objects::iLightSource *ptr);
	void RemoveLightSource(::Core::Objects::iLightSource *ptr);
	Object* CreateObject(const string& TypeName, Handle Parent, const string& Name = "");

	Object* SpawnObject(const string& TypeName, const string& Name, const Physics::vec3& Position);
	Object* SpawnObject_api(const string& TypeName, const string& Name, const math::vec3 &pos);
	Object* SpawnObjectChild(const string& TypeName, const string& Name, const Physics::vec3& Position, Handle Parent);
	Object* SpawnObjectChild_api(const string& TypeName, const string& Name, const math::vec3 &pos, Handle Parent);

//	Object* GetObjectByName(const string& Name);
//	const Objects::ObjectList& GetObjectsByName(const string& Name);
//	const Objects::ObjectList& GetObjectsByType(const string& Type);

	void ObjectDied(Handle h);

	const DataClasses::MapPtr& GetMapInstance() const { return m_MapData; }

	Objects::ObjectRegister* GetObjectRegister() { return m_Objects.get(); }

	static void RegisterScriptApi(::ApiInitializer &api);
//old
protected:
	string m_MapName;
	DataClasses::MapPtr m_MapData;
	Objects::UniqueObjectRegister m_Objects;
	std::list<Handle> m_DeadList;

	Physics::PhysicEnginePtr m_Physics;
	Graphic::Environment *m_Environment;
	Graphic::Light::LightConfiguration m_LightConfiguration;
	Handle m_PlayerHandle;

	virtual bool DoInitialize() override;
	virtual bool DoFinalize() override;
	virtual bool LoadMeta(const xml_node Node) override;
};

} // namespace Scene 
} // namespace Core 

#endif // GameScene_H_ 
