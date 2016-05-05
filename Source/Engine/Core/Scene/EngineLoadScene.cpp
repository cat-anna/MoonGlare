/*
	Generated by cppsrc.sh
	On 2014-12-17 21:05:35,24
	by Paweu
*/
#include <pch.h>
#include <MoonGlare.h>
#include <Engine/Core/DataManager.h>
#include <Engine/GUI/nGUI.h>
#include <Engine/GUI/Widgets/Panel.h>
#include <Engine/GUI/Widgets/Image.h>
#include <Engine/GUI/Widgets/Label.h>
#include "LoadingBaseScene.h"
#include "DefaultLoadingScene.h"
#include "EngineLoadScene.h"

#define ELSLoadingFirstSceneTitle	"EngineLoadScene_LoadingFirstScene"

namespace Core {
namespace Scene {

SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(EngineLoadScene)

EngineLoadScene::EngineLoadScene():
		BaseClass(),
		m_Aborted(false) {
	SetName(string("_") + this->GetDynamicTypeInfo()->Name);
}

EngineLoadScene::~EngineLoadScene() {
}

//----------------------------------------------------------------

void EngineLoadScene::LoadMain() {
	::OrbitLogger::ThreadInfo::SetName("ELS");
	AddLog(Info, "EngineLoadScene thread started");
	EnableScriptsInThisThread();

	GetDataMgr()->LoadGlobalData();
	GetDataMgr()->LoadPlayer();
	ASSERT(GetEngine()->GetPlayer());
	GetEngine()->GetPlayer()->Initialize();

	SetInfoText(ELSLoadingFirstSceneTitle);

	//for (;;)
		//std::this_thread::sleep_for(std::chrono::seconds(5));

	if (!m_Aborted)
		GetScenesManager()->AsyncSetNextScene(GetDataMgr()->GetConfiguration().m_FirstScene, GetEventProxy());
	else
		AddLog(Debug, "Engine load scene has been aborted");
	AddLog(Hint, "Loading thread finished!");
}

void EngineLoadScene::BeginScene() {
	THROW_ASSERT(IsInitialized() && !IsReady(), 0);

	m_LoadThread = GetEngine()->StartThread([this](){ LoadMain(); });

	SetReady(true);
}

void EngineLoadScene::EndScene() {
	THROW_ASSERT(IsReady(), 0);

	m_LoadThread.join();

	SetReady(false);
}

//----------------------------------------------------------------

int EngineLoadScene::InvokeOnTimer(int TimerID) { SCRIPT_INVOKE(OnTimer, TimerID); }
int EngineLoadScene::InvokeOnEscape() { 
	m_Aborted = true;
	SCRIPT_INVOKE(OnEscape); 
}
int EngineLoadScene::InvokeOnBeginScene() { SCRIPT_INVOKE(OnBeginScene); }
int EngineLoadScene::InvokeOnEndScene() { SCRIPT_INVOKE(OnEndScene); }
int EngineLoadScene::InvokeOnInitialize() { SCRIPT_INVOKE(OnInitialize); }
int EngineLoadScene::InvokeOnFinalize() { SCRIPT_INVOKE(OnFinalize); }

//----------------------------------------------------------------

int EngineLoadScene::InternalEventNotification(Events::InternalEvents event, int Param) {
	switch (event) {
	case ::Core::Events::InternalEvents::SceneLoadingFinished:
		AddLog(Hint, "Scene loading has been finished.");
		FinishScene();
		return 0;
	}
	return BaseClass::InternalEventNotification(event, Param);
}

} //namespace Scene 
} //namespace Core 
