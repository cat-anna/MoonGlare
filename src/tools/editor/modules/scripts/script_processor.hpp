#if 0
#pragma once

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
    void CheckReferences();
    void CheckProperties();
    void LoadScript();
	void Finalize();

	std::unique_ptr<lua_State, LuaWrap::LuaDeleter> m_Lua;
    std::string scriptString;
    std::string regName;
};

} //namespace Processor 
} //namespace Editor 
} //namespace MoonGlare

#endif
