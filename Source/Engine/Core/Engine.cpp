#include <pch.h>
#include <MoonGlare.h>

#include "Engine.h"
#include "Console.h"

#include <Graphic/Dereferred/DereferredPipeline.h>

#include <Renderer/RenderInput.h>
#include <Renderer/Frame.h>
#include <Source/Renderer/Renderer.h>

namespace MoonGlare {
namespace Core {

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(Engine);
RegisterApiDerivedClass(Engine, &Engine::ScriptApi);
RegisterApiInstance(Engine, &Engine::Instance, "Engine");
RegisterDebugApi(EngineDebug, &Engine::RegisterDebugScriptApi, "Debug");

Engine::Engine(World *world, Renderer::RendererFacade *Renderer) :
		cRootClass(),
		m_Flags(0),
		m_Running(false),

		m_LastFPS(0),
		m_FrameCounter(0),
		m_SkippedFrames(0),
		m_FrameTimeSlice(1.0f),

		m_Dereferred(),
		m_Forward(),

        m_World(world),
		m_Renderer(Renderer)
{
	MoonGlareAssert(m_World);
	MoonGlareAssert(m_Renderer);

	::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::Performance, "PERF");

	SetThisAsInstance();
	new JobQueue();
}

Engine::~Engine() {
	JobQueue::DeleteInstance();
}

//----------------------------------------------------------------------------------

bool Engine::Initialize() {

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

	return true;
}

bool Engine::PostSystemInit() {
	GetDataMgr()->LoadGlobalData();

	if (!m_World->PostSystemInit()) {
		AddLogf(Error, "World PostSystemInit failed!");
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------------

void Engine::ScriptApi(ApiInitializer &root){
	root
	.deriveClass<ThisClass, BaseClass>("cEngine")
		.addFunction("GetFrameRate", &ThisClass::GetFrameRate)
		.addFunction("GetInfoString", Utils::Template::InstancedStaticCall<ThisClass, string>::get<&ThisClass::GetVersionString>())

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
	if (!m_World->PreSystemStart()) {
		AddLogf(Error, "Failure during PreSystemStart");
		return;
	}

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
		conf.m_Frame = m_Renderer->NextFrame();
		float StartTime = static_cast<float>(glfwGetTime());

		conf.TimeDelta = CurrentTime - LastMoveTime;
		dev.GetContext()->Process();
		DoMove(conf);

		float MoveTime = static_cast<float>(glfwGetTime());

		conf.m_RenderInput->m_CommandQueues.Sort();

		float SortTime = static_cast<float>(glfwGetTime());

		DoRender(conf);

		float RenderTime = static_cast<float>(glfwGetTime());

		dev.EndFrame();

		float EndTime = static_cast<float>(glfwGetTime());
		LastMoveTime = CurrentTime;

		m_Renderer->Submit(conf.m_Frame);

		conf.m_SecondPeriod = CurrentTime - TitleRefresh >= 1.0;
		if(conf.m_SecondPeriod) {
			TitleRefresh = CurrentTime;
			m_LastFPS = m_FrameCounter;
			m_FrameCounter = 0;
			float sum = EndTime - StartTime;
			//if (Config::Current::EnableFlags::ShowTitleBarDebugInfo) {
				sprintf(Buffer, "time:%.2fs  fps:%u  frame:%llu  skipped:%u  mt:%.1f st:%.1f rti:%.1f swp:%.1f sum:%.1f fill:%.1f",
						CurrentTime, m_LastFPS, dev.FrameIndex(), m_SkippedFrames,
						(MoveTime - StartTime) * 1000.0f,
						(SortTime - MoveTime) * 1000.0f,
						(RenderTime - SortTime) * 1000.0f,
						(EndTime - RenderTime) * 1000.0f,
						(sum) * 1000.0f,
						(sum / m_FrameTimeSlice) * 100.0f
						);
				AddLogf(Performance, Buffer);
				dev.GetContext()->SetTitle(Buffer);
			//}
		}
	}

	if (!m_World->PreSystemShutdown()) {
		AddLogf(Error, "Failure during PreSystemShutdown");
		return;
	}
}

//----------------------------------------------------------------------------------

void Engine::DoRender(MoveConfig &conf) {
	auto &dev = *Graphic::GetRenderDevice();
	auto devsize = dev.GetContext()->Size();

	conf.m_RenderInput->OnBeginFrame(dev);
	dev.DispatchContextManipRequests();

	dev.BeginFrame();
	dev.ClearBuffer();

	using Renderer::RendererConf::CommandQueueID;
	conf.m_RenderInput->m_CommandQueues[CommandQueueID::PrepareFrame].Execute();

	if(conf.Camera)
		dev.Bind(conf.Camera);

	m_Dereferred->Execute(conf, dev);

	m_Forward->BeginFrame(dev);

	dev.SetModelMatrix(math::mat4());
	if (dev.CurrentEnvironment())
		dev.CurrentEnvironment()->Render(dev);

	m_Forward->BeginD2Render(dev);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	if(ConsoleExists()) GetConsole()->RenderConsole(dev);

	using Renderer::RendererConf::CommandQueueID;
	conf.m_RenderInput->m_CommandQueues[CommandQueueID::GUI].Execute();

	for (auto *it : conf.CustomDraw)
		it->D2Draw(dev);

	glDisable(GL_BLEND);

#ifdef DEBUG
	Config::Debug::ProcessTextureIntrospector(dev);
#endif
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 1);
	m_Forward->EndFrame();

	conf.CustomDraw.clear();
	conf.m_RenderInput->OnEndFrame();

	//dev.EndFrame();
}

void Engine::DoMove(MoveConfig &conf) {
	GetScriptEngine()->Step(conf);
	GetWorld()->Step(conf);
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

} //namespace Core
} //namespace MoonGlare
