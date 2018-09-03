#pragma once

#include <optional>

#include <Foundation/iFileSystem.h>

namespace MoonGlare::Renderer::Resources::Shader {

struct ShaderFileCache {
	using ReadBuffer = std::vector<std::string>;

    ShaderFileCache(iFileSystem *fs) : fileSystem(fs) {};

	bool ReadFile(const std::string &FName, const ReadBuffer *&out);
	void Clear() {
        loadedFiles.clear();
	}
    iFileSystem* GetFileSystem() {
        return fileSystem;
    }
private:
	std::unordered_map<std::string, std::optional<ReadBuffer>> loadedFiles;
    iFileSystem *fileSystem;
};

struct OutputBuffer : public std::list<std::string> {
    template<typename ... ARGS>
    void pushf(const char *fmt, ARGS&& ... args) {
        char buf[4 * 1024];
        sprintf_s(buf, fmt, std::forward<ARGS>(args)...);
        push_back(buf);
    }
    void pushs(const char *fmt, const std::string &str) {
        pushf(fmt, str.c_str());
    }
    template<typename ... ARGS>
    void push(const char *fmt, ARGS ... args) {
        push_back(fmt::format(fmt, std::forward<ARGS>(args)...));
    }
    void push(std::string l) {
        push_back(std::move(l));
    }
};                         

class Preprocessor final {
public:
 	Preprocessor(ShaderFileCache *fs);
 	~Preprocessor();

	struct ParseException {
		std::string m_FileName;
		std::string m_Message;
		int m_IncludeLevel;
	};
    struct MissingFileException {
        std::string m_FileName;
        int m_IncludeLevel;
    };

    void PushFile(const std::string &Name);
	void GetOutput(std::string &Output);

	void ClearOutput();
	void Clear();

	void Define(std::string id, std::string value) { defines[id] = value; }
	void Define(std::string id, const char* value) { defines[id] = value; }
	template<typename T> void Define(std::string id, T t) { Define(id, std::to_string(t)); }

	void DefineString(std::string id, std::string value) { Define(id, '"' + value + '"'); }
	void DefineString(std::string id, const char* value) { DefineString(id, value); }
	template<typename T> void DefineString(std::string id, T t) { DefineString(id, std::to_string(t)); }
private: 
	using ReadBuffer = ShaderFileCache::ReadBuffer;

    ShaderFileCache *fileCache;
	std::unordered_map<std::string, std::string> defines;
	std::unordered_map<std::string, bool> includedFiles;

	OutputBuffer outputBuffer;

	void Process(const std::string &FName, int level);
	void GenerateDefines();

	using PreprocessorTokenHadler = void(Preprocessor::*)(const std::string &FName, const std::string &line, int level, std::smatch match);

	void IncludeToken(const std::string &FName, const std::string &line, int level, std::smatch match);

	struct PreprocessorToken {
		std::regex regExp;
		PreprocessorTokenHadler handler;
	};
	static const std::array<PreprocessorToken, 1> s_Tokens;
};

} //namespace MoonGlare::Asset::Shader 
