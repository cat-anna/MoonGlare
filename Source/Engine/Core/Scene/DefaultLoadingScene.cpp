/*
 * cLoadingScene.cpp
 *
 *  Created on: 17-12-2013
 *      Author: Paweu
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

namespace Core {
namespace Scene {

SPACERTTI_IMPLEMENT_CLASS(DefaultLoadingScene)
RegisterApiDerivedClass(DefaultLoadingScene, &DefaultLoadingScene::RegisterScriptApi);
SceneClassRegister::Register<DefaultLoadingScene> SceneReg;

DefaultLoadingScene::DefaultLoadingScene() {
}

DefaultLoadingScene::~DefaultLoadingScene() {
}

//----------------------------------------------------------------------------------

void DefaultLoadingScene::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cDefaultLoadingScene")
	.endClass();
}

//----------------------------------------------------------------------------------

bool DefaultLoadingScene::DoInitialize() {
	if (!BaseClass::DoInitialize())
		return false;

	if (!SetDefaultLayout()) {
		AddLog(Error, "Unable to set default layout");
		return false;
	}

	return true;
}

bool DefaultLoadingScene::DoFinalize() {
	return BaseClass::DoFinalize();
}

void DefaultLoadingScene::SetInfoText(const string &TableEntryName) {
	if (!m_GUI)
		return;
	auto label = dynamic_cast<GUI::Widgets::Label*>(m_GUI->FindWidget(InfoLabelName()));
	if (!label)
		return;
	label->SetUtf8Text(GetDataMgr()->GetString(TableEntryName, InternalStringTable));
}

//----------------------------------------------------------------

int DefaultLoadingScene::InvokeOnTimer(int TimerID) { SCRIPT_INVOKE(OnTimer, TimerID); }
int DefaultLoadingScene::InvokeOnEscape() { SCRIPT_INVOKE(OnEscape); }
int DefaultLoadingScene::InvokeOnBeginScene() { SCRIPT_INVOKE(OnBeginScene); }
int DefaultLoadingScene::InvokeOnEndScene() { SCRIPT_INVOKE(OnEndScene); }
int DefaultLoadingScene::InvokeOnInitialize() { SCRIPT_INVOKE(OnInitialize); }
int DefaultLoadingScene::InvokeOnFinalize() { SCRIPT_INVOKE(OnFinalize); }

//----------------------------------------------------------------

bool DefaultLoadingScene::SetDefaultLayout() {
	m_GUI = std::make_unique<GUI::GUIEngine>();
	m_GUI->Initialize(Graphic::GetRenderDevice()->GetContext().get());
	GetGUI()->GetRootWidget()->SetStyleByName("DefaultLoadingStyle");

	auto root = GetGUI()->GetRootWidget();
	root->Lock();

	auto *lpanel = new GUI::Widgets::Panel(root);
	lpanel->SetAlignMode(GUI::AlignMode::Left);
	lpanel->SetSize(root->GetSize() / 2.0f);
	root->AddWidget(lpanel);

	auto *rpanel = new GUI::Widgets::Panel(root);
	rpanel->SetAlignMode(GUI::AlignMode::Right);
	rpanel->SetSize(root->GetSize() / 2.0f);
	root->AddWidget(rpanel);

	root->RecalculateMetrics();

	auto *img = new GUI::Widgets::Image(root);
	img->SetAlignMode(GUI::AlignMode::Center);
	img->SetImage("MoonClouds");
	img->SetName("MoonImage");
	lpanel->AddWidget(img);

	auto *text = new GUI::Widgets::Label(rpanel);
	text->SetAlignMode(GUI::AlignMode::LeftMiddle);
	text->SetTextAlignMode(GUI::TextAlignMode::LeftMiddle);
	text->SetUtf8Text("Loading...");
	text->SetName(InfoLabelName());
	rpanel->AddWidget(text);

	root->Unlock();
	root->RecalculateMetrics();

	return true;
}

} //namespace Scene 
} //namespace Core 
