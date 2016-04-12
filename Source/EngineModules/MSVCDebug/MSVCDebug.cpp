/*
  * Generated by cppsrc.sh
  * On 2015-07-16 22:48:40,19
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <nfMoonGlare.h>
#include <Engine/ModulesManager.h>
#include "MSVCDebug.h"
#include <Windows.h>

namespace MoonGlare {
namespace Debug {

struct MSVCDebuggerOutputPolicy {
	void Write(const OrbitLogger::LogLine *line, const char *c) {
		OutputDebugStringA(c);
	}
};

struct MSVCDebuggerFormatPolicy {
	void Format(const OrbitLogger::LogLine *line, char* buffer, size_t buffer_size) {
		auto src = line->m_SourceInfo;

		const char* file = src->m_File;
		
		//static const char *skipstr = "d:\\programowanie\\projekty\\!gry\\moonglare\\";
		//static const int skip = strlen(skipstr);
		//if (!memcmp(skipstr, src->m_File, skip))
		//	file += skip;

		sprintf_s(buffer, buffer_size, "%s(%d) : %s : %s\n", file, src->m_Line, line->m_ModeStr, line->m_Message);
	}
};

struct MSVCDebuggerFilteringPolicy {
	bool Filter(const OrbitLogger::LogLine *line) const { return line->m_SourceInfo != 0; }
};

using MSVCDebuggerLogSink = OrbitLogger::LogSink < MSVCDebuggerOutputPolicy, MSVCDebuggerFormatPolicy, MSVCDebuggerFilteringPolicy > ;

//----------------------------------------------------------------
 
struct MSVCDebugModule : public MoonGlare::Modules::ModuleInfo {
	MSVCDebugModule(): BaseClass("MSVCDebug", ModuleType::Debug) { }

	virtual bool Initialize() override {
		if (!IsDebuggerPresent())
			return false;
		new MSVCDebug();
		OrbitLogger::LogCollector::AddLogSink<MSVCDebuggerLogSink>();
		return true;
	}
	virtual bool Finalize() override {
		if (!MSVCDebug::InstanceExists())
			return true;
		MSVCDebug::DeleteInstance();
		return true;
	}
private:
};
DEFINE_MODULE(MSVCDebugModule);

//----------------------------------------------------------------

GABI_IMPLEMENT_CLASS_SINGLETON(MSVCDebug);
RegisterApiDerivedClass(MSVCDebug, &MSVCDebug::RegisterScriptApi);

MSVCDebug::MSVCDebug() {
	SetThisAsInstance();
}

MSVCDebug::~MSVCDebug() {
}

//----------------------------------------------------------------

void MSVCDebug::RegisterScriptApi(::ApiInitializer &root) {
	root
	.deriveClass<ThisClass, BaseClass>("cMSVCDebug")
	.endClass()
	;
}

//----------------------------------------------------------------

void MSVCDebug::DebuggerPrint(const char* line) {
	OutputDebugStringA(line);
}

void MSVCDebug::DebuggerPrint(const string& line) {
	OutputDebugStringA(line.c_str());
}

} //namespace Debug 
} //namespace MoonGlare 
