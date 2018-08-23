#include "Preprocessor.h"

#include <functional>

namespace MoonGlare::Renderer::Resources::Shader {

bool ShaderFileCache::ReadFile(const std::string &FName, const ReadBuffer *&out) {
    auto it = loadedFiles.find(FName);
    if (it != loadedFiles.end()) {
        if (!it->second.has_value()) {
            AddLogf(Warning, "Unable to load file '%s'", FName.c_str());
            return false;
        }
        out = &it->second.value();
        return true;
    }

    StarVFS::ByteTable data;
    if (!GetFileSystem()->OpenFile(data, "file:///Shaders/" + FName)) {
        AddLogf(Warning, "Unable to load file '%s'", FName.c_str());
        return false;
    }

    ReadBuffer buf;
    std::stringstream ss;
    ss << data.c_str();
    while (ss) {
        std::string line;
        std::getline(ss, line);
        buf.emplace_back(std::move(line));
    }

    loadedFiles[FName] = std::move(buf);
    out = &loadedFiles[FName].value();

    return true;
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

const std::array<Preprocessor::PreprocessorToken, 1> Preprocessor::s_Tokens = {
    PreprocessorToken{ std::regex(R"(^\s*#include\s+[<"]([/A-Za-z0-9_\.]+)[">]\s*)"), &Preprocessor::IncludeToken },
};

Preprocessor::Preprocessor(ShaderFileCache *fc):
    fileCache(fc) {
}

Preprocessor::~Preprocessor() {
}

void Preprocessor::PushFile(const std::string & Name) {
    try {
        if (outputBuffer.empty())
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
    for (auto line : outputBuffer)
        ss << line << std::endl;

    Output = ss.str();
}

void Preprocessor::ClearOutput() {
    includedFiles.clear();
    outputBuffer.clear();
}

void Preprocessor::Clear() {
    ClearOutput();
    defines.clear();
}

void Preprocessor::Process(const std::string &FName, int level) {
    if (includedFiles.find(FName) != includedFiles.end()) {
        outputBuffer.pushf("//@ skip[%d] %s - included", level, FName.c_str());
        return;
    }

    includedFiles[FName] = true;
        
    const ReadBuffer *lines;
    if (!fileCache->ReadFile(FName, lines)) {
        AddLogf(Error, "Unable to load file '%s'", FName.c_str());
        throw ParseException{ FName , "Unable to load file", level };
    }

    outputBuffer.pushf("//@ start[%d] %s", level, FName.c_str());

    unsigned linenum = 0;
    try {
        outputBuffer.push(fmt::format("#line {}", 1));
        for(const auto &line : *lines) {
            ++linenum;

            bool handled = false;
            std::smatch match;
            for (auto token : s_Tokens) {
                if (std::regex_search(line, match, token.regExp)) {

                    outputBuffer.pushs("//@ %s", line);

                    (this->*token.handler)(FName, line, level, std::move(match));

                    outputBuffer.push(fmt::format("#line {}", linenum));

                    handled = true;
                    break;
                }
            }

            if (!handled) {
                outputBuffer.push_back(line);
            }
        }
    }
    catch (ParseException &e) {
        AddLogf(Error, "Error while processing included file %s at line %d", FName.c_str(), linenum);
        throw e;
    }

    outputBuffer.pushf("//@ end[%d] %s", level, FName.c_str());
}

void Preprocessor::GenerateDefines() {
    for (auto &it : defines) {
        std::string line;
        line = "#define ";
        line += it.first;
        line += " ";
        line += it.second;
        outputBuffer.push_back(line);
    }
}

void Preprocessor::IncludeToken(const std::string &FName, const std::string &line, int level, std::smatch match) {
    std::string iName = match[1];
    try {
        outputBuffer.pushf("//@ pause[%d] %s", level, FName.c_str());
        Process(iName, level + 1);
        outputBuffer.pushf("//@ continue[%d] %s", level, FName.c_str());
    }
    catch (ParseException &) {
        AddLogf(Error, "Error while processing included file %s", iName.c_str());
        throw;
    }
}

} //namespace MoonGlare::Asset::Shader
