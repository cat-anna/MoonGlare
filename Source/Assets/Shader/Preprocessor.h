/*
  * Generated by cppsrc.sh
  * On 2017-02-04 20:57:07,50
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include <boost/optional.hpp>

namespace MoonGlare::Asset::Shader {

struct FileCache {
	using ReadBuffer = std::vector<std::string>;

	FileCache(FileSystem *fs) :m_FileSystem(fs) {};

	bool ReadFile(const std::string &FName, const ReadBuffer *&out);
	void Clear() {
		m_LoadedFiles.clear();
	}
private:
	std::unordered_map<std::string, boost::optional<ReadBuffer>> m_LoadedFiles;
	FileSystem *m_FileSystem;
};

class Preprocessor final {
public:
 	Preprocessor(FileCache *fs);
 	~Preprocessor();

	struct ParseException {
		std::string m_FileName;
		std::string m_Message;
		int m_IncludeLevel;
	};

	void PushFile(const std::string &Name) throw(ParseException);
	void GetOutput(std::string &Output);

	void ClearOutput();
	void Clear();

	void Define(std::string id, std::string value) { m_Defines[id] = value; }
	void Define(std::string id, const char* value) { m_Defines[id] = value; }
	template<typename T> void Define(std::string id, T t) { Define(id, std::to_string(t)); }

	void DefineString(std::string id, std::string value) { Define(id, '"' + value + '"'); }
	void DefineString(std::string id, const char* value) { DefineString(id, value); }
	template<typename T> void DefineString(std::string id, T t) { DefineString(id, std::to_string(t)); }
private: 
	using ReadBuffer = FileCache::ReadBuffer;

	FileCache *m_FileCache;
	std::unordered_map<std::string, std::string> m_Defines;
	std::unordered_map<std::string, bool> m_IncludedFiles;

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
        template<typename ...ARGS>
        void push(const char *fmt, ARGS ... args) {
            push_back(fmt::format(fmt, std::forward<ARGS>(args)...));
        }
	};
	OutputBuffer m_OutputBuffer;


	void Process(const std::string &FName, int level);
	void GenerateDefines();

	using PreprocessorTokenHadler = void(Preprocessor::*)(const std::string &FName, const std::string &line, int level, std::smatch match);

	void IncludeToken(const std::string &FName, const std::string &line, int level, std::smatch match);

	struct PreprocessorToken {
		std::regex m_RegExp;
		PreprocessorTokenHadler m_Handler;
	};
	static const std::array<PreprocessorToken, 1> s_Tokens;
};

} //namespace MoonGlare::Asset::Shader 
