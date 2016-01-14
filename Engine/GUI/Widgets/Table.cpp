/*
  * Generated by cppsrc.sh
  * On 2015-03-19 21:29:57,04
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "../GUI.h"
#include "Table.h"
#include "Panel.h"

namespace MoonGlare {
namespace GUI {
namespace Widgets {

class TablePanel : public Panel {
	GABI_DECLARE_STATIC_CLASS(TablePanel, Panel)
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	TablePanel(iWidget *Parent): BaseClass(Parent) { }
	virtual ~TablePanel() { }

	virtual void RecalculateMetrics() override {
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

GABI_IMPLEMENT_STATIC_CLASS(TablePanel);
RegisterApiDerivedClass(TablePanel, &TablePanel::RegisterScriptApi);

//----------------------------------------------------------------------------------

GABI_IMPLEMENT_STATIC_CLASS(Table);
RegisterApiDerivedClass(Table, &Table::RegisterScriptApi);
WidgetClassRegister::Register<Table> TablelReg;

Table::Table(iWidget *Parent):
		BaseClass(Parent),
		m_CellCount(0, 0),
		m_MaxCellIndex(0) {
}

Table::~Table() {
}

//----------------------------------------------------------------------------------

void Table::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("TableWidget")
	.endClass()
	;
}

void Table::ScriptSet(Utils::Scripts::TableDispatcher &table) {
	//Lock();
	//BaseClass::ScriptSet(table);
	//m_Children.Clean();
	//try {
	//	auto td = table.GetTable("Children");
	//	m_Children.CreateChildren(td, this);
	//}
	//catch (...) { /* ignore */ }
	//Unlock();
	//RecalculateMetrics();
	LOG_NOT_IMPLEMENTED;
}

//----------------------------------------------------------------------------------

void Table::Process(const Core::MoveConfig &conf) {
	for (auto &it : m_Table)
		it->Process(conf);
}

void Table::Draw(Graphic::MatrixStack &dev){
	dev.Push();
	dev.Translate(m_Position);
	for (auto &it : m_Table)
		it->Draw(dev);
	dev.Pop();
}

//----------------------------------------------------------------------------------

void Table::SetCellCount(UnsignedPoint point) {
	m_Table.clear();

	m_MaxCellIndex = point[0] * point[1];
	m_CellCount = point;
	m_Table.resize(m_MaxCellIndex);
	if (!m_MaxCellIndex) return;
	BaseClass::RecalculateMetrics(); //recalculate metrics for this widget
	auto CellSize = GetSize();
	CellSize[0] /= (float)m_CellCount[0];
	CellSize[1] /= (float)m_CellCount[1];

	for (unsigned i = 0; i < m_MaxCellIndex; ++i) {
		auto p = std::make_unique<TablePanel>(this);
		p->Lock();
		auto location = Point(IndexToLocation(i));
		p->SetSize(CellSize);
		p->SetPosition(location * CellSize);
		p->SetAlignMode(AlignMode::Parent);

		p->Unlock();
		p->RecalculateMetrics();
		m_Table[i].swap(p);
	}
}

Panel* Table::GetPanelByIndex(unsigned index) const {
	if (m_MaxCellIndex <= index) {
		AddLog(Error, "Attempt to get nonexisting panel index! [" << index << "]");
		return nullptr;
	}
	return m_Table[index].get();
}

void Table::SetWidget(Widget &w, unsigned index) {
	if (m_MaxCellIndex <= index) {
		AddLog(Error, "Attempt to set nonexisting panel index! [" << index << "]");
		return;
	}
	auto *p = m_Table[index].get();
	p->CleanWidgets();
	p->AddWidget(w);
}

void Table::RecalculateMetrics() {
	BaseClass::RecalculateMetrics();
	for (auto &it : m_Table)
		it->RecalculateMetrics();
}

//----------------------------------------------------------------------------------

void Table::SetStyle(SharedStyleSet style) {
	BaseClass::SetStyle(style);
	for (auto &it : m_Table)
		it->SetStyle(style);
}

//----------------------------------------------------------------------------------

void Table::Lock() {
	if (IsLocked()) return;
	BaseClass::Lock();
	for (auto &it : m_Table) it->Lock();
}

void Table::Unlock() {
	BaseClass::Unlock();
	if (!IsLocked())
		for (auto &it : m_Table) it->Unlock();
}	

iWidget* Table::FindWidget(const string& name) const {
	if (GetName() == name)
		return const_cast<ThisClass*>(this);

	for (auto &it : m_Table) {
		auto ptr = it->FindWidget(name);
		if (ptr) return ptr;
	}

	return nullptr;
}

iWidget* Table::GetWidgetAt(const Point &p) const {
	if (!m_BoundingRect.IsPointInside(p))
		return nullptr;
	
	for (auto &it : m_Table) {
		auto *w = it->GetWidgetAt(p);
		if (w)
			return w;
	}

	return nullptr;
}

} //namespace Widgets 
} //namespace GUI 
} //namespace MoonGlare 

