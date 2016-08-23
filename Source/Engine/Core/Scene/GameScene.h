/*
 * GameScene.h
 *
 *  Created on: 17-12-2013
 *      Author: Paweu
 */

#ifndef GameScene_H_
#define GameScene_H_

namespace MoonGlare {
namespace Core {
namespace Scene {

class GameScene : public ciScene {
	SPACERTTI_DECLARE_CLASS(GameScene, ciScene)
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

	Physics::PhysicEngine& GetPhysicsEngine() { return *m_Physics; }

	Object* GetObjectByName(const string& Name);
//	const Objects::ObjectList& GetObjectsByName(const string& Name);
//	const Objects::ObjectList& GetObjectsByType(const string& Type);

	Objects::ObjectRegister* GetObjectRegister() { return &m_Objects; }

	static void RegisterScriptApi(::ApiInitializer &api);
//old
protected:
	string m_MapName;
	Objects::ObjectRegister m_Objects;
	std::list<Handle> m_DeadList;

	Physics::PhysicEnginePtr m_Physics;
	Graphic::Environment m_Environment;
	Handle m_PlayerHandle;

	virtual bool DoInitialize() override;
	virtual bool DoFinalize() override;
	virtual bool LoadMeta(const xml_node Node) override;
};

} // namespace Scene 
} // namespace Core 
} //namespace MoonGlare 

#endif // GameScene_H_ 
