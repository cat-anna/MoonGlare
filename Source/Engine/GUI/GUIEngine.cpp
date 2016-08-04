/*	
  * Generated by cppsrc.sh
  * On 2015-03-01  7:01:47,55
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "GUI.h"
#include <Core/InputMap.h>
//#include <Engine/Graphic/nGraphic.h>

namespace MoonGlare {
namespace GUI {

struct EventDispatcherTable {
	using EventFunc = void(GUIEngine::*)(Events::Event &e);

	EventFunc m_Table[(unsigned)Events::EventType::MaxValue];

	void Dispatch(GUIEngine *engine, Events::EventType type, Events::Event &e) const {
		EventFunc f = m_Table[(unsigned)type];
		(engine->*f)(e);
	}

	EventDispatcherTable() {
		for (unsigned i = 0; i < (unsigned)Events::EventType::MaxValue; ++i)
			m_Table[i] = &GUIEngine::EventUnknownAction;

#define _set(Name, Function) m_Table[(unsigned)Events::EventType::Name] = (EventFunc)&GUIEngine::Function

		_set(MouseDown, EventMouseDownAction);
		_set(MouseUp, EventMouseUpAction);

#undef _set
	}
};

const EventDispatcherTable EventDispatcher;

//----------------------------------------------------------------------------------

struct GuiMouseDispatcher : public ::Core::MouseEventDispatcher {
	GuiMouseDispatcher(GUIEngine *owner) : m_Owner(owner) { }

	~GuiMouseDispatcher() { }

	virtual void MouseUpEvent(MouseButton Button, ModsStatus Modificators) override { 
		Events::MouseUpEvent event;
		event.Type = Events::MouseUpEvent::StaticType();
		event.Button = Button;
		event.Mods = Modificators;
		m_Owner->PostEvent(event);
	};
	virtual void MouseDownEvent(MouseButton Button, ModsStatus Modificators) override { 
		Events::MouseDownEvent event;
		event.Type = Events::MouseDownEvent::StaticType();
		event.Button = Button;
		event.Mods = Modificators;
		m_Owner->PostEvent(event);
	};

	GUIEngine *m_Owner;
};

//----------------------------------------------------------------------------------

SPACERTTI_IMPLEMENT_STATIC_CLASS(GUIEngine)
//SPACERTTI_IMPLEMENT_CLASS_SINGLETON(GUIEngine)
RegisterApiDerivedClass(GUIEngine, &GUIEngine::RegisterScriptApi);
inline GUIEngine* CurrentInstance() { return Core::GetEngine()->GetCurrentScene()->GetGUI().get(); }
RegisterApiDynamicInstance(GUIEngine, &CurrentInstance, "GetGUI");

GUIEngine::GUIEngine():
		BaseClass(),
		m_Window(0),
		m_RootWidget(),
		m_FocusedWidget(0),
		m_Flags(0) {
}

GUIEngine::~GUIEngine() {
}

//----------------------------------------------------------------------------------

void GUIEngine::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cGUIEngine")
		.addFunction("FindWidget", &ThisClass::FindWidget)
		.addFunction("GetRootWidget", &ThisClass::GetRootWidget)
		.addFunction("GetCursor", &ThisClass::GetCursor)
		.addFunction("ConstructGUI", Utils::Scripts::TableCallDispatcher<ThisClass, &ThisClass::ConstructGUI>::get())
	.endClass()
	;
}

//----------------------------------------------------------------------------------

void GUIEngine::ConstructGUI(Utils::Scripts::TableDispatcher &table) {
	if (m_RootWidget)
		m_RootWidget->ScriptSet(table);
}

//----------------------------------------------------------------------------------

void GUIEngine::Activate() {
	//ReleaseInstance();
	//SetThisAsInstance();
	if (m_MouseObserver)
		m_MouseObserver->Set();
}

void GUIEngine::Deactivate() {
	//ReleaseInstance();
	if (m_MouseObserver)
		m_MouseObserver->Revoke();
}

bool GUIEngine::Initialize(Graphic::Window *Window) {
	m_Window = Window;
	m_StyleManager = std::make_unique<StyleManager>(this);
	m_RootWidget = std::make_unique<Screen>(this, Window);
	m_RootWidget->SetStyleByName("Default");
	return true;
}

bool GUIEngine::Finalize() {
	SetFocus(nullptr);
	if (m_Mouse)
		m_Mouse->Finalize();
	m_Mouse.reset();
	m_RootWidget.reset();
	m_StyleManager.reset();
	return true;
}

iWidget* GUIEngine::FindWidget(const string& name) const {
	if (!m_RootWidget) return nullptr;
	return m_RootWidget->FindWidget(name);
}

void GUIEngine::EnableCursor(bool state) {
	SetCursorInputEnabled(state);
	if (!state) {
		if (m_Mouse)
			m_Mouse->Finalize();
		m_Mouse.reset();
		m_MouseObserver.reset();
	} else {
		m_Mouse = std::make_unique<Mouse>(this);
		m_Mouse->Initialize();
		m_MouseObserver = std::make_unique<GuiMouseDispatcher>(this);
	}
}

//----------------------------------------------------------------------------------

void GUIEngine::Draw(Graphic::cRenderDevice &dev) {
	if (!m_RootWidget)
		return;
	Graphic::MatrixStack stack(dev);
	if (m_RootWidget->IsVisible())
		m_RootWidget->Draw(stack);
	if (m_Mouse)
		m_Mouse->Draw(stack);
}

void GUIEngine::Process(const Core::MoveConfig &conf) {
	DispatchPendingEvents();
	if (m_Mouse)
		m_Mouse->Process(conf);
	if (m_RootWidget)
		m_RootWidget->Process(conf);
}

//----------------------------------------------------------------------------------

void GUIEngine::SetFocus(iWidget *widget) {
	if (m_FocusedWidget == widget)
		return;

	if (m_FocusedWidget)
		m_FocusedWidget->SetFocused(false);

	m_FocusedWidget = widget;

	if (m_FocusedWidget)
		m_FocusedWidget->SetFocused(true);
}

void GUIEngine::SetHovered(iWidget *widget) {
	if (m_Mouse)
		m_Mouse->SetHovered(widget);
}

//----------------------------------------------------------------------------------

void GUIEngine::DispatchPendingEvents() {
	if (m_EventQueue.empty())
		return;// there is nothing to do
/*
	decltype(m_EventQueue) queue;
	m_EventQueueMutex.lock();
	queue.swap(m_EventQueue);
	m_EventQueueMutex.unlock();

	while (!queue.empty()) {

	}
*/
	//int enable = 0;
	while (!m_EventQueue.empty()) {
		//if (enable <= 0)
			//break;
		//--enable;

		try {
			auto *ptr = m_EventQueue.front();
			if (!ptr)
				break;
			EventDispatcher.Dispatch(this, ptr->Type, *ptr);
			m_EventQueue.pop();
		}
		catch (...) {
			AddLog(Error, "An error has occur during processing of gui event!");
		}
	}
}

void GUIEngine::EventUnknownAction(Events::Event &e) {
	AddLog(Warning, "Catched unknown event of id: " << (unsigned)e.Type << " and type name: '" << typeid(e).name() << "'");
}

void GUIEngine::EventMouseUpAction(Events::MouseUpEvent &e) {
	AddLog(Debug, "dispatching mouse up event");
	if (m_Mouse) m_Mouse->OnMouseUp(e);
}

void GUIEngine::EventMouseDownAction(Events::MouseDownEvent &e) {
	AddLog(Debug, "dispatching mouse down event");
	if (m_Mouse) m_Mouse->OnMouseDown(e);
}

//----------------------------------------------------------------------------------

} //namespace GUI 
} //namespace MoonGlare 
