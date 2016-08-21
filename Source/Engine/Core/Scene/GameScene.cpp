/*
 * GameScene.cpp
 *
 *  Created on: 17-12-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>
#include <Engine/Core/DataManager.h>
#include <Engine/GUI/nGUI.h>

namespace MoonGlare {
namespace Core {
namespace Scene {

SPACERTTI_IMPLEMENT_CLASS(GameScene);
RegisterApiDerivedClass(GameScene, &GameScene::RegisterScriptApi);
SceneClassRegister::Register<GameScene> GameSceneReg;

GameScene::GameScene():
		BaseClass(),
		m_MapName(),
		m_Objects(GetEngine()->GetWorld(), this),
		m_Physics(),
		m_Environment() {
	m_Physics = std::make_unique<Physics::PhysicEngine>();
	m_Camera = std::make_unique<Camera::iCamera>(this);
}

GameScene::~GameScene() {
	m_Objects.Clear();
	//delete m_Physics;
}
 
//---------------------------------------------------------------------------------------

void GameScene::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cGameScene")
		.addFunction("GetObjectByName", &ThisClass::GetObjectByName)
//		.addFunction("GetObjectsByName", &ThisClass::GetObjectsByName)
//		.addFunction("GetObjectsByType", &ThisClass::GetObjectsByType)
	.endClass();
}

//---------------------------------------------------------------------------------------

void GameScene::BeginScene() {
	THROW_ASSERT(IsInitialized() && !IsReady(), 0);

	Graphic::GetRenderDevice()->BindEnvironment(&m_Environment);
	m_Environment.Initialize();

	BaseClass::BeginScene();
}

void GameScene::EndScene() {
	THROW_ASSERT(IsReady(), 0);

	m_Environment.Finalize();
	Graphic::GetRenderDevice()->BindEnvironment(nullptr);

	BaseClass::EndScene();
}

bool GameScene::DoInitialize() {
	if (!BaseClass::DoInitialize())
		return false;

	m_GUI = std::make_unique<GUI::GUIEngine>();
	m_GUI->Initialize(Graphic::GetRenderDevice()->GetContext().get());

	m_Objects.LoadObjects(GetRootNode().child("Objects"), this);
	m_Objects.InitializeObjects();

	return true;
} 

bool GameScene::DoFinalize() {
	if (m_GUI) m_GUI->Finalize();
	m_GUI.reset();
	return BaseClass::DoFinalize();
}
 
//----------------------------------------------------------------
 
bool GameScene::LoadMeta(const xml_node Node) {
	BaseClass::LoadMeta(Node);
	m_Environment.LoadMeta(Node.child("Environment"));
	return true;
}

//----------------------------------------------------------------

void GameScene::DoMove(const MoveConfig &conf) {
	BaseClass::DoMove(conf);
	m_Objects.Process(conf);

	//struct T {
	//	static bool t(btManifoldPoint& cp, void* body0,void* body1) {
	//		if (cp.m_userPersistentData)
	//			return false;
	//
	//		cp.m_userPersistentData = body0;
	//		btCollisionObject *b0 = (btCollisionObject*)body0;
	//		btCollisionObject *b1 = (btCollisionObject*)body1;
	//		Object *o0 = (Object*)b0->getUserPointer();
	//		Object *o1 = (Object*)b1->getUserPointer();
	//
	//		AddLog(Hint, "contact " << b0 << "@" << o0->GetName() << "   " << b1 << "@" << o1->GetName());
	//		return false;
	//	}
	//	static bool destroy(void* userPersistentData) {
	//		return true;
	//	}
	//};
	//gContactProcessedCallback = &T::t;
	//gContactDestroyedCallback = &T::destroy;

	//AddLog(Hint, "Step begin");
	//auto t = std::chrono::steady_clock::now();
	//m_Physics->Step(conf.TimeDelta);
	//if (conf.m_SecondPeriod) {
	//	std::chrono::duration<double> sec = std::chrono::steady_clock::now() - t;
	//	AddLogf(Performance, "ph step: %f ms", (float)(sec.count() * 1000));
	//}

	//AddLog(Hint, "Step end");

//	conf.m_LightConfig->DirectionalLights.push_back(m_Environment.GetAmbientLight());

	if (m_Camera)
		m_Camera->Update(conf);
}

#if 0
Graphic::Light::LightConfiguration* GameScene::GetLightConfig() {
	static Graphic::Light::SpotLight sl;
	if (m_LightConfiguration.SpotLights.empty()) {
		sl.Direction = glm::normalize(math::vec3(1, 0, 1));
		sl.CutOff = 0.7f;
		sl.Position = math::vec3(5, 0, 5);
		sl.Attenuation.Exp = 0.3f;
		sl.Attenuation.Linear = 0.7f;
		sl.AmbientIntensity = 1.0f;
		sl.Precalculate();
		m_LightConfiguration.SpotLights.push_back(&sl);
	}
	return &m_LightConfiguration;
}
#endif // 0

//----------------------------------------------------------------

Object* GameScene::GetObjectByName(const string& Name) { return m_Objects.GetFirstObjectByName(Name); }
//const Objects::ObjectList& GameScene::GetObjectsByName(const string& Name) { return m_Objects->GetObjectsByName(Name); }
//const Objects::ObjectList& GameScene::GetObjectsByType(const string& Type) { return m_Objects->GetObjectsByType(Type); }

//----------------------------------------------------------------

int GameScene::InvokeOnTimer(int TimerID) { SCRIPT_INVOKE(OnTimer, TimerID); }
int GameScene::InvokeOnEscape() { SCRIPT_INVOKE(OnEscape); }
int GameScene::InvokeOnBeginScene() { SCRIPT_INVOKE(OnBeginScene); }
int GameScene::InvokeOnEndScene() { SCRIPT_INVOKE(OnEndScene); }
int GameScene::InvokeOnInitialize() { SCRIPT_INVOKE(OnInitialize); }
int GameScene::InvokeOnFinalize() { SCRIPT_INVOKE(OnFinalize); }

} // namespace Scene
} // namespace Core
} // namespace MoonGlare 
