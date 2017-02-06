/*
  * Generated by cppsrc.sh
  * On 2017-02-04 20:57:07,50
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include "../FileSystem.h"
#include "Preprocessor.h"

#include <functional>

namespace MoonGlare::Asset::Shader {

const std::array<Preprocessor::PreprocessorToken, 1> Preprocessor::s_Tokens = {
	PreprocessorToken{ std::regex(R"(^\s*#include\s+[<"]([A-Za-z0-9_\.]+)[">]\s*)"), &Preprocessor::IncludeToken },
};

Preprocessor::Preprocessor(FileSystem *fs):
	m_FileSystem(fs) {
}

Preprocessor::~Preprocessor() {
}

void Preprocessor::PushFile(const std::string & Name) {
	try {
		if (m_OutputBuffer.empty())
			GenerateDefines();

		Process(Name, 0);
	}
	catch (ParseException &e) {
		AddLogf(Error, "Error while processing file %s", Name.c_str());
		throw e;
	}

}

void Preprocessor::GetOutput(std::string &Output) {
	std::stringstream ss;
	for (auto line : m_OutputBuffer)
		ss << line << std::endl;

	Output = ss.str();
}

void Preprocessor::ClearOutput() {
	m_IncludedFiles.clear();
	m_OutputBuffer.clear();
}

void Preprocessor::Clear() {
	ClearOutput();
	m_Defines.clear();
}

bool Preprocessor::ReadFile(const std::string &FName, ReadBuffer &out) {
	out.clear();
	StarVFS::ByteTable data;
	if (!GetFileSystem()->OpenFile(FName, DataPath::Shaders, data)) {
		AddLogf(Error, "Unable to load file '%s' for shader '%s' ", FName.c_str(), FName.c_str());
		return false;
	}

	std::stringstream ss;
	ss << data.c_str();
	while (ss) {
		std::string line;
		std::getline(ss, line);
		out.emplace_back(std::move(line));
	}

	return true;
}

void Preprocessor::Process(const std::string &FName, int level) {
	if (m_IncludedFiles.find(FName) != m_IncludedFiles.end()) {
		m_OutputBuffer.pushf("//@ skip[%d] %s - included", level, FName.c_str());
		return;
	}

	m_IncludedFiles[FName] = true;
		
	ReadBuffer lines;
	if (!ReadFile(FName, lines)) {
		AddLogf(Error, "Unable to load file '%s'", FName.c_str());
		throw ParseException{ FName , "Unable to load file", level };
	}

	m_OutputBuffer.pushf("//@ start[%d] %s", level, FName.c_str());

	unsigned linenum = 0;
	try {
		while (!lines.empty()) {
			auto line = lines.front();
			lines.pop_front();
			++linenum;

			bool handled = false;
			std::smatch match;
			for (auto token : s_Tokens) {
				if (std::regex_search(line, match, token.m_RegExp)) {
					m_OutputBuffer.pushs("//@ %s", line);
					(this->*token.m_Handler)(FName, line, level, std::move(match));
					handled = true;
					break;
				}
			}

			if (!handled) {
				m_OutputBuffer.push_back(line);
			}
		}
	}
	catch (ParseException &e) {
		AddLogf(Error, "Error while processing included file %s at line %d", FName.c_str(), linenum);
		throw e;
	}

	m_OutputBuffer.pushf("//@ end[%d] %s", level, FName.c_str());
}

void Preprocessor::GenerateDefines() {
	for (auto &it : m_Defines) {
		std::string line;
		line = "#define ";
		line += it.first;
		line += " ";
		line += it.second;
		m_OutputBuffer.push_back(line);
	}
}

void Preprocessor::IncludeToken(const std::string &FName, std::string & line, int level, std::smatch match) {
	std::string iName = match[1];
	try {
		m_OutputBuffer.pushf("//@ pause[%d] %s", level, FName.c_str());
		Process(iName, level + 1);
		m_OutputBuffer.pushf("//@ continue[%d] %s", level, FName.c_str());
	}
	catch (ParseException &e) {
		AddLogf(Error, "Error while processing included file %s", iName.c_str());
		throw;
	}
}

} //namespace MoonGlare::Asset::Shader
