/*
  * Generated by cppsrc.sh
  * On 2016-10-13 18:53:51,09
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef ScriptProcessor_H
#define ScriptProcessor_H

namespace MoonGlare {
namespace Editor {
namespace Processor {

struct ScriptFileProcessorInfo;

struct ScriptFileProcessor
	: public QtShared::iFileProcessor {

	ScriptFileProcessor(ScriptFileProcessorInfo *Module, std::string URI);

	virtual ProcessResult ProcessFile() override;
private:
    ScriptFileProcessorInfo *module;

	void InitLua();
    void ExecuteScript();
	void ProcessOutput();
	void Finalize();

	std::unique_ptr<lua_State, LuaWrap::LuaDeleter> m_Lua;
};

} //namespace Processor 
} //namespace Editor 
} //namespace MoonGlare 

#endif
