#include <pch.h>
#include <MoonGlare.h>

#include "Engine.h"

#include <GUI/GUI.h>
#include "Console.h"
#include <Core/InputMap.h>

#include <Graphic/Dereferred/DereferredPipeline.h>

#include <Renderer/RenderInput.h>

namespace MoonGlare {
namespace Core {

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(Engine);
RegisterApiDerivedClass(Engine, &Engine::ScriptApi);
RegisterApiInstance(Engine, &Engine::Instance, "Engine");
RegisterDebugApi(EngineDebug, &Engine::RegisterDebugScriptApi, "Debug");

Engine::Engine() :
		cRootClass(),
		m_Flags(0),
		m_Running(false),

		m_LastFPS(0),
		m_FrameCounter(0),
		m_SkippedFrames(0),
		m_FrameTimeSlice(1.0f),

		m_CurrentScene(0),
		m_Dereferred(),
		m_Forward()
{
	::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::Performance, "PERF");

	SetThisAsInstance();
	new JobQueue();
	new ::Core::Input();
}

Engine::~Engine() {
	::Core::Input::DeleteInstance();
	JobQueue::DeleteInstance();
}

//----------------------------------------------------------------------------------

bool Engine::Initialize() {
	m_World = std::make_unique < World >();

	if (!m_World->Initialize(GetScriptEngine())) {
		AddLogf(Error, "Failed to initialize world!");
		return false;
	}

	m_Dereferred = std::make_unique<Graphic::Dereferred::DereferredPipeline>();
	m_Forward = std::make_unique<Graphic::Forward::ForwardPipeline>();

	m_Dereferred->Initialize();
	m_Forward->Initialize();

	SetFrameRate((float)Graphic::GetRenderDevice()->GetContext()->GetRefreshRate());

	return true;
}

bool Engine::Finalize() {
	m_Dereferred.reset();
	m_Forward.reset();

	AddLog(Performance, "Frames skipped: " << m_SkippedFrames);

	if (!m_World->Finalize()) {
		AddLogf(Error, "Failed to finalize world!");
	}
	m_World.reset();

	return true;
}

//----------------------------------------------------------------------------------

void Engine::ScriptApi(ApiInitializer &root){
	root
	.deriveClass<ThisClass, BaseClass>("cEngine")
		.addFunction("GetFrameRate", &ThisClass::GetFrameRate)
		.addFunction("GetInfoString", Utils::Template::InstancedStaticCall<ThisClass, string>::get<&ThisClass::GetVersionString>())
		
		.addFunction("SetNextScene", &ThisClass::SetNextScene)
		.addFunction("ClearSceneStack", &ThisClass::ClearSceneStack)
		.addFunction("PopScenes", &ThisClass::PopScenes)
		.addFunction("ClearScenesUntil", &ThisClass::ClearScenesUntil)

//		.addFunction("CaptureScreenShot", &ThisClass::CaptureScreenShot)
		
#ifdef DEBUG_SCRIPTAPI
		.addFunction("SetFrameRate", &ThisClass::SetFrameRate)
#endif
	.endClass()
	;
}

#if DEBUG_SCRIPTAPI

void Engine::RegisterDebugScriptApi(ApiInitializer &root){ }

#endif

//----------------------------------------------------------------------------------

void Engine::Exit() {
	m_Running = false;
}

void Engine::Abort() {
	m_Running = false;
	m_ActionQueue.Add([]{ throw __FUNCTION__ " called!"; });
	throw __FUNCTION__ " called!";
}

void Engine::EngineMain() {
	m_CurrentScene = GetScenesManager()->GetNextScene();
	if (m_CurrentScene)
		m_CurrentScene->BeginScene();
	Graphic::GetRenderDevice()->GetContext()->GrabMouse();

	m_Running = true;
	char Buffer[256];

	Graphic::cRenderDevice &dev = *Graphic::GetRenderDevice();
	float CurrentTime = static_cast<float>(glfwGetTime());
	float TitleRefresh = 0.0;
	float LastFrame = CurrentTime;
	float LastMoveTime = CurrentTime;

	MoveConfig conf;
	conf.m_RenderInput = dev.CreateRenderInput();

	while (m_Running) {
		CurrentTime = static_cast<float>(glfwGetTime());
		float FrameTimeDelta = CurrentTime - LastFrame;
		if (FrameTimeDelta < m_FrameTimeSlice) {
	//		std::this_thread::sleep_for(std::chrono::microseconds(100));
			std::this_thread::yield();
			continue;
		}

		if (FrameTimeDelta >= m_FrameTimeSlice * 1.5f) {
			//LastFrame = CurrentTime;
			++m_SkippedFrames;
			//std::this_thread::sleep_for(std::chrono::microseconds(500));
			//std::this_thread::yield();
			//continue;
		}

		LastFrame = CurrentTime;
#if 0
		if(m_Flags & AppFlag_Inactive){
			std::this_thread::yield();
			LastMoveTime = CurrentTime;
			continue;
		}
#endif
		m_ActionQueue.DispatchPendingActions();

		++m_FrameCounter;
		float StartTime = static_cast<float>(glfwGetTime());

		conf.TimeDelta = CurrentTime - LastMoveTime;
		dev.GetContext()->Process();
		DoMove(conf);

		float MoveTime = static_cast<float>(glfwGetTime());
		DoRender(conf);

		float RenderTime = static_cast<float>(glfwGetTime());

		dev.EndFrame();

		float EndTime = static_cast<float>(glfwGetTime());
		LastMoveTime = CurrentTime;

		conf.m_SecondPeriod = CurrentTime - TitleRefresh >= 1.0;
		if(conf.m_SecondPeriod) {
			TitleRefresh = CurrentTime;
			m_LastFPS = m_FrameCounter;
			m_FrameCounter = 0;
			float sum = EndTime - StartTime;
			//if (Config::Current::EnableFlags::ShowTitleBarDebugInfo) {
				sprintf(Buffer, "time:%.2fs  fps:%u  frame:%llu  skipped:%u  mt:%.1f rti:%.1f swp:%.1f sum:%.1f fill:%.1f", 
						CurrentTime, m_LastFPS, dev.FrameIndex(), m_SkippedFrames, 
						(MoveTime - StartTime) * 1000.0f,
						(RenderTime - MoveTime) * 1000.0f,
						(EndTime - RenderTime) * 1000.0f,
						(sum) * 1000.0f,
						(sum / m_FrameTimeSlice) * 100.0f
						);
				AddLogf(Performance, Buffer);
				dev.GetContext()->SetTitle(Buffer);
			//}
		}
	}

	if (m_CurrentScene)
		m_CurrentScene->EndScene();
}         

void Engine::HandleEscapeKeyImpl() {
	if (!m_CurrentScene)
		return; //actin cannot be handled properly so ignore

	if (m_CurrentScene->InvokeOnEscape())
		return; //Action handled nothing to do more

	AddLog(Debug, "Unhandled escape key catched!");
#ifdef DEBUG
	Exit();
#endif
}

//----------------------------------------------------------------------------------

void Engine::ChangeSceneImpl() {
	if (m_CurrentScene)
		m_CurrentScene->EndScene();
	auto *prevScene = m_CurrentScene;
	m_CurrentScene = GetScenesManager()->GetNextScene();
	if (m_CurrentScene) 
		m_CurrentScene->BeginScene();
	AddLogf(Hint, "Changed scene from '%s'[%p] to '%s'[%p]",
			(prevScene ? prevScene->GetName().c_str() : "NULL"), prevScene,
			(m_CurrentScene ? m_CurrentScene->GetName().c_str() : "NULL"), m_CurrentScene);
	if (prevScene)
		GetScenesManager()->PushScene(prevScene);
}

//----------------------------------------------------------------------------------

void Engine::DoRender(MoveConfig &conf) {
	auto &dev = *Graphic::GetRenderDevice();
	auto devsize = dev.GetContext()->Size();

	conf.m_RenderInput->OnBeginFrame(dev);
	dev.DispatchContextManipRequests();
	
	dev.BeginFrame();
	dev.ClearBuffer();

	if(conf.Camera)
		dev.Bind(conf.Camera);
	 
	if (conf.Scene)
		m_Dereferred->Execute(conf, dev);
	m_Forward->BeginFrame(dev);

	dev.SetModelMatrix(math::mat4());
	if (dev.CurrentEnvironment())
		dev.CurrentEnvironment()->Render(dev);
	  
	m_Forward->BeginD2Render(dev);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	if(ConsoleExists()) GetConsole()->RenderConsole(dev);
	if (m_CurrentScene && m_CurrentScene->GetGUI())
		m_CurrentScene->GetGUI()->Draw(dev);
	glDisable(GL_BLEND);
#ifdef DEBUG   
	Config::Debug::ProcessTextureIntrospector(dev);
#endif
	glActiveTexture(GL_TEXTURE0);   
	glBindTexture(GL_TEXTURE_2D, 1);
	m_Forward->EndFrame();
	conf.m_RenderInput->OnEndFrame();

	//dev.EndFrame();
} 

void Engine::DoMove(MoveConfig &conf) {
	conf.Scene = nullptr;

	m_TimeEvents.CheckEvents(conf);
	GetScriptEngine()->Step(conf);
	GetWorld()->Step(conf);
	if (m_CurrentScene)
		m_CurrentScene->DoMove(conf);
}

//----------------------------------------------------------------------------------

void Engine::SetFrameRate(float value) {
	if (value < 1.0f) 
		value = 1.0f;
	else {
		float refresh = (float)Graphic::GetRenderDevice()->GetContext()->GetRefreshRate();
		if (value > refresh)
			value = refresh;
	}

	m_FrameTimeSlice = 1.0f / value;
	AddLogf(Debug, "Frame rate limit set to %d (%.5f ms per frame)", (unsigned)value, m_FrameTimeSlice * 1000.0f);
}

//----------------------------------------------------------------------------------

std::thread Engine::StartThread(std::function<void()> ThreadMain) {
	return std::thread([ThreadMain]() {
		try {
			ThreadMain();
			return;
		}
		catch (const char * Msg){
			AddLogf(Error, "FATAL ERROR! '%s'", Msg);
		}
		catch (const string & Msg){
			AddLogf(Error, "FATAL ERROR! '%s'", Msg.c_str());
		}
		catch (std::exception &E){
			AddLog(Error, "FATAL ERROR! '" << E.what() << "'");
		}
		catch (...){
			AddLog(Error, "UNKNOWN FATAL ERROR!");
		}
		try {
			GetEngine()->Abort();
		} catch (...) { }
	});
}

//----------------------------------------------------------------------------------

string Engine::GetVersionString() {
#ifdef DEBUG
	return ::MoonGlare::Core::GetMoonGlareEngineVersion().VersionStringFull();
#else
	return ::MoonGlare::Core::GetMoonGlareEngineVersion().VersionString();
#endif
}

//----------------------------------------------------------------------------------

void Engine::SetNextScene(const string& Name) const { 
	GetScenesManager()->SetNextScene(Name); 
}

void Engine::ClearSceneStack() {
	GetScenesManager()->ClearSceneStack();
}

void Engine::PopScenes(int count) const {
	GetScenesManager()->PopScenes(count);
}

void Engine::ClearScenesUntil(const string& Name) const {
	GetScenesManager()->ClearScenesUntil(Name);
}


} //namespace Core
} //namespace MoonGlare 
