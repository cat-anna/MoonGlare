/*
 * Debug.cpp
 *
 * Source file for debug configuration
 *
 *  Created on: 03-08-2013
 *      Author: Paweu
 */
#include <pch.h>
#ifdef DEBUG
#include <MoonGlare.h>

namespace Config {
namespace Debug {

bool EnableFlags::Physics = true;
bool EnableFlags::PhysicsDebugDraw = false;

bool EnableFlags::ShowTitleBarDebugInfo = true;

//---------------------------------------------------------------------------------------

void DebugThrow(const char* msg) {
	throw std::runtime_error(msg);
}

void DebugSleep(int ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void ScriptDebug(ApiInitializer &root){
	root
	.addFunction("Dump", &DumpRuntime)
	.addFunction("Throw", &DebugThrow)
	.addFunction("Sleep", &DebugSleep)
	.beginNamespace("Flags")
		.beginNamespace("Physics")
			.addVariable("Enabled", &EnableFlags::Physics)
			.addVariable("DebugDraw", &EnableFlags::PhysicsDebugDraw)
		.endNamespace()
		.addVariable("ShowTitleBarDebugInfo", &EnableFlags::ShowTitleBarDebugInfo)
	.endNamespace();
	;
}

RegisterDebugApi(ScriptDebug, &ScriptDebug, "Debug");

//---------------------------------------------------------------------------------------

volatile bool _ThreadCanContinue;

void DebugThread() {
#if 0
	while (_ThreadCanContinue) {
		PROCESS_MEMORY_COUNTERS pmc;
		if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
			AddLogf(Hint, "Memory usage: %.3f MB", pmc.PeakWorkingSetSize / (1024.0f * 1024.0f));
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
#endif // 0
}

void Initialize() {
	_ThreadCanContinue = true;
	//std::thread(&DebugThread).detach();
#ifdef _BUILDING_ENGINE_
#endif
}
void Finalize() {
	_ThreadCanContinue = false;
}

//---------------------------------------------------------------------------------------
void Debug::DumpRuntime() {
	REQUIRE_REIMPLEMENT;
//#ifndef _DISABLE_LOG_SYSTEM_
	//GabiLib::GabiTracker::DumpInstances(__CreateLog(Debug));
//#endif
}

#ifdef _BUILDING_ENGINE_
void Debug::DumpAll(const char* prefixname) {
	char buf[256];
	GabiLib::GabiTracker::SortList();

	std::ofstream Instances((sprintf(buf, "logs/%s_Instances.txt", prefixname), buf));
	GabiLib::GabiTracker::DumpInstances(Instances);

	std::ofstream ClassInfo((sprintf(buf, "logs/%s_ClassInfo.txt", prefixname), buf));
	GabiLib::GabiTracker::DumpClasees(ClassInfo);

	std::ofstream ClassLists((sprintf(buf, "logs/%s_ClassLists.txt", prefixname), buf));
	::Core::Interfaces::DumpLists(ClassLists);
}

void Debug::CheckInstances() {
	bool hdr = false;
	for (auto it = GabiLib::GabiTracker::begin(), jt = GabiLib::GabiTracker::end(); it != jt; ++it) {
		if ((*it)->InstancesCount == 0)
			continue;
		if (!hdr) {
			AddLog(Error, "MEMORY LEAK!!!");
			AddLog(Error, "Remain instances:");
			hdr = true;
		}
		AddLogf(Error, "remain: %3d Class: '%s' ", (*it)->InstancesCount, (*it)->GetFullName());
	}
}
#endif 

//---------------------------------------------------------------------------------------
		
void eAsserationError::DescribeSender(std::stringstream &ss, const cRootClass *sender) {
	ss << "Sender: " << sender->Info() << "\n";
}

//---------------------------------------------------------------------------------------

#ifdef _FEATURE_TEXTURE_INTROSCPECTOR_
struct TextureInstrospectorData {
	int TexIndex = -1;
	bool Initialized = false;
	Graphic::VAO vao;
};

TextureInstrospectorData TextureInstrospector;

void TextureInstrospectorControll(Utils::Scripts::TableDispatcher &table) {
	int next = table.getInt("next", 0);
	if (next != 0)
		TextureInstrospector.TexIndex += next;
	else
		TextureInstrospector.TexIndex = table.getInt("id", -1);

	AddLogf(Hint, "TextureInstrospector id:%d", TextureInstrospector.TexIndex);
}

void TextureInstrospectorApi(ApiInitializer &root) {
	root
		.addFunction("TextureInstrospector", Utils::Scripts::TableStaticCallDispatcher::get<&TextureInstrospectorControll>())
	;
}

RegisterDebugApi(TextureDebug, &TextureInstrospectorApi);

void ProcessTextureIntrospector(Graphic::cRenderDevice &dev) {
//#if 0
	if (TextureInstrospector.TexIndex >= 0) {
		dev.SetModelMatrix(math::mat4());
		dev.CurrentShader()->SetBackColor(math::vec3(1));
		 
		if (!TextureInstrospector.Initialized) {
			TextureInstrospector.Initialized = true;
			auto size = math::fvec2(dev.GetContextSize());
			float h = size[0];
			float w = size[1];
			float hh = h / 2.0f;
			float hw = w / 2.0f; 
			Graphic::VertexVector coords =  {
				{hw, hh, 0},
				{ w, hh, 0},
				{ w,  h, 0},
				{hw,  h, 0},
			};  
			Graphic::TexCoordVector tex =  {
				{0, 1},
				{1, 1}, 
				{1, 0}, 
				{0, 0},
			}; 
			Graphic::IndexVector index ={ 0, 1, 2, 3, }; 
			Graphic::NormalVector normals;
			TextureInstrospector.vao.DelayInit(coords, tex, normals, index);
		}
		TextureInstrospector.vao.Bind(); 
		glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, 0);
		//TextureInstrospector.vao.DrawElements(4, 0, 0, GL_QUADS);
		glBindTexture(GL_TEXTURE_2D, TextureInstrospector.TexIndex);
		//TextureInstrospector.vao.DrawElements(4, 0, 0, GL_QUADS);
	}
//#endif
}
#endif
} //namespace Debug
} //namespace Config

#endif
