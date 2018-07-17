#pragma once

#include <map>
#include <string>

class EditorTab;
class CodeEditor;

struct CompilationResult {
	enum class Status {
		Compiling, Success, Failed, Error,
	} status;
	std::map<unsigned, std::string> lineInfo;
};

class LuaLinter {
public:
    LuaLinter();
 	~LuaLinter();
    CompilationResult Compile(const std::string & code);
};
