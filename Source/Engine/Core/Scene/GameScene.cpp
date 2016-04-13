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

namespace Core {
namespace Scene {

GABI_IMPLEMENT_CLASS(GameScene);
RegisterApiDerivedClass(GameScene, &GameScene::RegisterScriptApi);
SceneClassRegister::Register<GameScene> GameSceneReg;

GameScene::GameScene():
		BaseClass(),
		m_MapName(),
		m_MapData(),
		m_Objects(),
		m_Physics(),
		m_Environment(0) {
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
		.addFunction("InvokeOnTimer", &ThisClass::InvokeOnTimer)

		.addFunction("CreateObject", &ThisClass::CreateObject)
		.addFunction("SpawnObject", &ThisClass::SpawnObject_api)
		.addFunction("SpawnObjectChild", &ThisClass::SpawnObjectChild_api)
		//.addFunction("SpawnObjectXYZ", Utils::Template::DynamicArgumentConvert<ThisClass, Physics::vec3, &ThisClass::SpawnObject_api, float, float, float>::get())

//		.addFunction("GetObjectByName", &ThisClass::GetObjectByName)
//		.addFunction("GetObjectsByName", &ThisClass::GetObjectsByName)
//		.addFunction("GetObjectsByType", &ThisClass::GetObjectsByType)
	.endClass();
}

//---------------------------------------------------------------------------------------

void GameScene::BeginScene() {
	THROW_ASSERT(IsInitialized() && !IsReady(), 0);

	auto player = GetEngine()->GetPlayer().get();
	if (player) {
		player->SetOwnerScene(this);
		m_PlayerHandle = m_Objects.Insert(std::unique_ptr<Object>(player));
		//player->SetPosition(Physics::vec3(0, 0.6, 0));
	} else {
		AddLog(Error, "There is no player instance!");
	}

	if (m_MapData) {
		m_Environment = m_MapData->GetEnvironment();
		if (m_Environment) {
			Graphic::GetRenderDevice()->BindEnvironment(m_Environment);
			m_Environment->Initialize();
			m_LightConfiguration.DirectionalLights.push_back(&m_Environment->GetAmbientLight());
		}
	}

	BaseClass::BeginScene();
}

void GameScene::EndScene() {
	THROW_ASSERT(IsReady(), 0);

	//do finalize objects in scene?
	auto player = GetEngine()->GetPlayer().get();
	if (player) {
		m_Objects.Release(m_PlayerHandle);
		player->SetOwnerScene(nullptr);
	} else {
		AddLog(Error, "There is no player instance!");
	}

	if (m_Environment) {
		m_Environment->Finalize();
		m_LightConfiguration.DirectionalLights.remove(&m_Environment->GetAmbientLight());
	}
	Graphic::GetRenderDevice()->BindEnvironment(nullptr);

	BaseClass::EndScene();
}

bool GameScene::DoInitialize() {
	if (!BaseClass::DoInitialize())
		return false;

	m_GUI = std::make_unique<GUI::GUIEngine>();
	m_GUI->Initialize(Graphic::GetRenderDevice()->GetContext().get());

	if (m_MapData) {
		m_Objects.Insert(m_MapData->LoadMapObject());
	}

	m_Objects.InitializeObjects();

	return true;
} 

bool GameScene::DoFinalize() {
	if (m_MapData) {
		m_MapData->Finalize();
	}
	if (m_GUI) m_GUI->Finalize();
	m_GUI.reset();
	return BaseClass::DoFinalize();
}
 
//----------------------------------------------------------------
 
bool GameScene::LoadMeta(const xml_node Node) {
	BaseClass::LoadMeta(Node);

	const char *MapName = Node.child(xmlRes_SceneMap).text().as_string(0);
	if (!MapName) {
		AddLog(Warning, "Game scene xml does not have specified map!");
	} else {
		m_MapName = MapName;
	}

	if (!m_MapData && !m_MapName.empty()) {
		m_MapData = GetDataMgr()->GetMap(m_MapName);
		if (m_MapData){
			m_MapData->SetOwnerScene(this);
			m_MapData->LoadMeta();
			m_MapData->LoadMapObjects(m_Objects);
			if (!m_MapData->Initialize()){
				AddLogf(Error, "Unable to initialize map '%s' for game scene '%s'", m_MapName.c_str(), GetName().c_str());
				return false;
			}
		} else {
			AddLogf(Error, "Unable to load map '%s' for game scene '%s'", m_MapName.c_str(), GetName().c_str());
			return false;
		}
	}

	m_Objects.LoadObjects(Node.child("Objects"), this);

	return true;
}

//----------------------------------------------------------------
 
void GameScene::AddLightSource(iLightSource *ptr) {
	using Graphic::Light::LightType;
	switch (ptr->GetType()) {
		case LightType::Point:{
			auto lptr = dynamic_cast<Graphic::Light::PointLight*>(ptr->GetRawPointer());
			if (!lptr) {
				AddLog(Error, "Light type and pointer does not match!");
				return;
			}
			m_LightConfiguration.PointLights.push_back(lptr);
			break;
		}
		case LightType::Spot:{
			auto lptr = dynamic_cast<Graphic::Light::SpotLight*>(ptr->GetRawPointer());
			if (!lptr) {
				AddLog(Error, "Light type and pointer does not match!");
				return;
			}
			m_LightConfiguration.SpotLights.push_back(lptr);
			break;
		}		
		case LightType::Directional:{
			auto lptr = dynamic_cast<Graphic::Light::DirectionalLight*>(ptr->GetRawPointer());
			if (!lptr) {
				AddLog(Error, "Light type and pointer does not match!");
				return;
			}
			m_LightConfiguration.DirectionalLights.push_back(lptr);
			break;
		}
		default:
			AddLog(Error, "Unknown light type!");
			return;
	}
}

void GameScene::RemoveLightSource(iLightSource *ptr) {
	using Graphic::Light::LightType;
	auto lptr = ptr->GetRawPointer();
	switch (ptr->GetType()) {
		case LightType::Point:
			m_LightConfiguration.PointLights.remove((Graphic::Light::PointLight*)lptr);
			break;
		case LightType::Spot:
			m_LightConfiguration.SpotLights.remove((Graphic::Light::SpotLight*)lptr);
			break;
		case LightType::Directional:
			m_LightConfiguration.DirectionalLights.remove((Graphic::Light::DirectionalLight*)lptr);
			break;
		default:
			AddLog(Error, "Unknown light type!");
			return;
	}
}

void GameScene::ObjectDied(Handle h) { 
	ASSERT_HANDLE_TYPE(Object, h);
	m_DeadList.push_back(h); 
}

Object* GameScene::CreateObject(const string& TypeName, Handle Parent, const string& Name) {
	Object *obj = GetDataMgr()->LoadObject(TypeName, this, Parent);
	if (!obj) {
		AddLogf(Error, "Unable to create object of name '%s'", TypeName.c_str());
		return 0;
	}
	obj->SetOwnerScene(this);
	obj->Initialize();
	obj->SetName(Name);
	return obj;
}

Object* GameScene::SpawnObject(const string& TypeName, const string& Name, const Physics::vec3& Position) {
	auto *o = CreateObject(TypeName, m_Objects.GetRootHandle(), Name);
	if (!o)
		return nullptr;
	o->SetPosition(Position);
	o->UpdateMotionState();
	AddLog(Debug, "Created object '" << TypeName << "' of name '" << Name << "' at " << Position);
	return o;
}

Object* GameScene::SpawnObject_api(const string& TypeName, const string& Name, const math::vec3 &pos) {
	return SpawnObject(TypeName, Name, convert(pos));
}

Object* GameScene::SpawnObjectChild(const string& TypeName, const string& Name, const Physics::vec3& Position, Handle Parent) {
	auto *o = CreateObject(TypeName, Parent, Name);
	if (!o)
		return nullptr;

	//auto pptr = m_Objects.Get(Parent);

	o->SetPosition(Position/* + pptr->GetPosition()*/);
	o->UpdateMotionState();
	AddLog(Debug, "Created object child '" << TypeName << "' of name '" << Name << "' at " << Position);
	return o;
}

Object* GameScene::SpawnObjectChild_api(const string& TypeName, const string& Name, const math::vec3 &pos, Handle Parent) {
	return SpawnObjectChild(TypeName, Name, convert(pos), Parent);
}

//----------------------------------------------------------------

void GameScene::DoMove(const MoveConfig &conf) {
	BaseClass::DoMove(conf);

	m_Objects.Process(conf);

	struct T {
		static bool t(btManifoldPoint& cp, void* body0,void* body1) {
			if (cp.m_userPersistentData)
				return false;

			cp.m_userPersistentData = body0;
			btCollisionObject *b0 = (btCollisionObject*)body0;
			btCollisionObject *b1 = (btCollisionObject*)body1;
			Object *o0 = (Object*)b0->getUserPointer();
			Object *o1 = (Object*)b1->getUserPointer();

			AddLog(Hint, "contact " << b0 << "@" << o0->GetName() << "   " << b1 << "@" << o1->GetName());
			return false;
		}
		static bool destroy(void* userPersistentData) {
			return true;
		}
	};
	//gContactProcessedCallback = &T::t;
	//gContactDestroyedCallback = &T::destroy;

	//AddLog(Hint, "Step begin");
	auto t = std::chrono::steady_clock::now();
	m_Physics->Step(conf.TimeDelta);
	std::chrono::duration<double> sec = std::chrono::steady_clock::now() - t;
	static unsigned counter = 0;
	++counter;
	if ((counter & 0xF) == 0) {
		AddLogf(Performance, "pht: %f ms", (float)(sec.count() * 1000));
	}

	//AddLog(Hint, "Step end");

	if (!m_DeadList.empty()) {
		for (auto i : m_DeadList) {
			m_Objects.Remove(i);
		}
		m_DeadList.clear();
	}

	if (m_Camera)
		m_Camera->Update(conf);
}

Graphic::Light::LightConfiguration* GameScene::GetLightConfig() {
#if 0
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
#endif // 0
	return &m_LightConfiguration;
}

//----------------------------------------------------------------

//Object* GameScene::GetObjectByName(const string& Name) { return m_Objects->GetFirstObjectByName(Name); }
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
