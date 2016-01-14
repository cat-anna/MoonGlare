/*
  * Generated by cppsrc.sh
  * On 2015-05-21 21:00:20,91
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "../GUI.h"
#include "Panel.h"
#include "SplitPanel.h"

namespace MoonGlare {
namespace GUI {
namespace Widgets {

class SplittingPanel : public Panel {
	GABI_DECLARE_STATIC_CLASS(SplittingPanel, Panel)
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	SplittingPanel(iWidget *Parent): BaseClass(Parent) { }
	virtual ~SplittingPanel() { }

	virtual void RecalculateMetrics() override {
		AddLog(TODO, "Support for outer margin");
		m_BoundingRect.SliceFromParent(GetParent()->GetBoundingRect(), m_Position, m_Size);
		m_ScreenPosition = GetParent()->GetScreenPosition() + m_Position;
		for (auto &it : m_Children)
			it->RecalculateMetrics();
	}

	static void RegisterScriptApi(ApiInitializer &api) {
		api
		.deriveClass<ThisClass, BaseClass>("TablePanelWidget")
		.endClass()
		;
	}
};

GABI_IMPLEMENT_STATIC_CLASS(SplittingPanel);
RegisterApiDerivedClass(SplittingPanel, &SplittingPanel::RegisterScriptApi);

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

GABI_IMPLEMENT_STATIC_CLASS(SplitPanel);
RegisterApiDerivedClass(SplitPanel, &SplitPanel::RegisterScriptApi);
WidgetClassRegister::Register<SplitPanel> SplitPanelReg;

SplitPanel::SplitPanel(iWidget *Parent) :
		BaseClass(Parent),
		m_SplitPosition(0.5f),
		m_SplitPadding(0.0f) {
	m_LeftPanel = std::make_unique<SplittingPanel>(this);
	m_RightPanel = std::make_unique<SplittingPanel>(this);
}

SplitPanel::~SplitPanel() {
}

//----------------------------------------------------------------------------------

void SplitPanel::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("SplitPanelWidget")
	.endClass()
	;
}

void SplitPanel::ScriptSet(Utils::Scripts::TableDispatcher &table) {
	Lock();
	BaseClass::ScriptSet(table);
	
	if (table.ElementExists("SplitPosition")) m_SplitPosition = table.getFloat("SplitPosition", 0.5f);
	if (table.ElementExists("SplitPadding")) m_SplitPadding = table.getFloat("SplitPadding", 0.0f);
	if (table.ElementExists("SplitOrientation")) m_SplitOrientation = OrientationEnum::ConvertSafe(table.getString("SplitOrientation", ERROR_STR));
	if (table.IsTable("LeftPanel")) {
		auto t = table.GetTable("LeftPanel");
		m_LeftPanel->ScriptSet(t);
	}
	if (table.IsTable("RightPanel")) {
		auto t = table.GetTable("RightPanel");
		m_RightPanel->ScriptSet(t);
	}

	Unlock();
	RecalculateMetrics();
}

//----------------------------------------------------------------------------------

void SplitPanel::Process(const Core::MoveConfig &conf) {
	m_LeftPanel->Process(conf);
	m_RightPanel->Process(conf);
}

void SplitPanel::Draw(Graphic::MatrixStack &dev){
	dev.Push();
	dev.Translate(m_Position);
	m_LeftPanel->Draw(dev);
	m_RightPanel->Draw(dev);
	dev.Pop();
}

//----------------------------------------------------------------------------------

Panel* SplitPanel::GetLeftPanel() {
	return m_LeftPanel.get();
}

Panel* SplitPanel::GetRightPanel() {
	return m_RightPanel.get();
}

void SplitPanel::CleanWidgets() {
	m_LeftPanel->CleanWidgets();
	m_RightPanel->CleanWidgets();
	if (!IsLocked())
		RecalculateMetrics();
}

void SplitPanel::RecalculateMetrics() {
	BaseClass::RecalculateMetrics();

	Point leftsize = m_Size;
	Point rightsize = m_Size;
	Point rightpos(0.0f);
	float hpadding = m_SplitPadding / 2.0f;

	if (m_SplitOrientation == Orientation::Horizontal) {
		leftsize.y *= m_SplitPosition;
		leftsize.y -= hpadding;
		rightsize.y *= 1.0f - m_SplitPosition;
		rightsize.y -= hpadding;
		rightpos.y = leftsize.y + m_SplitPadding;
	} else {
		leftsize.x *= m_SplitPosition;
		leftsize.x -= hpadding;
		rightsize.x *= 1.0f - m_SplitPosition;
		rightsize.x -= hpadding;
		rightpos.x = leftsize.x + m_SplitPadding;
	}

	Lock();
	m_LeftPanel->SetPosition(Point(0));
	m_LeftPanel->SetSize(leftsize);
	m_RightPanel->SetPosition(rightpos);
	m_RightPanel->SetSize(rightsize);
	Unlock();

	m_LeftPanel->RecalculateMetrics();
	m_RightPanel->RecalculateMetrics();
}

//----------------------------------------------------------------------------------

void SplitPanel::SetStyle(SharedStyleSet style) {
	BaseClass::SetStyle(style);
	m_LeftPanel->SetStyle(style);
	m_RightPanel->SetStyle(style);
}

//----------------------------------------------------------------------------------

void SplitPanel::Lock() {
	if (IsLocked()) return;
	BaseClass::Lock();
	m_LeftPanel->Lock();
	m_RightPanel->Lock();
}

void SplitPanel::Unlock() {
	BaseClass::Unlock();
	if (!IsLocked()) {
		m_LeftPanel->Unlock();
		m_RightPanel->Unlock();
	}
}

iWidget* SplitPanel::FindWidget(const string& name) const {
	if (GetName() == name)
		return const_cast<ThisClass*>(this);

	auto ret = m_LeftPanel->FindWidget(name);
	if (ret) return ret;

	return m_RightPanel->FindWidget(name);
}

iWidget* SplitPanel::GetWidgetAt(const Point &p) const {
	if (!m_BoundingRect.IsPointInside(p))
		return nullptr;
	
	auto ret = m_LeftPanel->GetWidgetAt(p);
	if (ret) return ret;

	return m_RightPanel->GetWidgetAt(p);
}

//----------------------------------------------------------------------------------

void SplitPanel::SetSplitPosition(float value) {
	m_SplitPosition = value;
	if (!IsLocked())
		RecalculateMetrics();
}

void SplitPanel::SetSplitPadding(float value) {
	m_SplitPadding = value;
	if (!IsLocked())
		RecalculateMetrics();
}

void SplitPanel::SetSplitOrientation(Orientation value) {
	m_SplitOrientation = value;
	if (!IsLocked())
		RecalculateMetrics();
}

} //namespace Widgets 
} //namespace GUI 
} //namespace MoonGlare 
