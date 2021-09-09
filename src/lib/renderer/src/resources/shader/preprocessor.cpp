#include "preprocessor.hpp"
#include <functional>
#include <orbit_logger.h>
#include <sstream>

namespace MoonGlare::Renderer::Resources::Shader {

void ShaderFileCache::Clear() {
    loaded_files.clear();
}

void ShaderFileCache::ReadFile(const std::string &file_name,
                               const ShaderFileCache::ReadBuffer *&out) {
    if (auto it = loaded_files.find(file_name); it != loaded_files.end()) {
        out = it->second.get();
        return;
    }

    std::string data;
    if (!file_system->ReadFileByPath(file_name, data)) {
        throw std::runtime_error("Failed to load file " + file_name);
    }

    ReadBuffer buf;
    std::stringstream ss{data};
    while (ss) {
        std::string line;
        std::getline(ss, line);
        buf.emplace_back(std::move(line));
    }

    loaded_files[file_name] = std::make_unique<ReadBuffer>(move(buf));
    out = loaded_files[file_name].get();
}

//---------------------------------------------------------------------------------------

const std::array<Preprocessor::PreprocessorToken, 1> Preprocessor::kKnownTokens = {
    PreprocessorToken{std::regex(R"==(^\s*#include\s+[<"]([/A-Za-z0-9_\.]+)[">]\s*)=="),
                      &Preprocessor::HandleIncludeToken},
};

//---------------------------------------------------------------------------------------

void Preprocessor::PushFile(const std::string &name) {
    try {
        if (output_buffer.empty()) {
            GenerateDefines();
        }
        Process(name, 0);
    } catch (const ParseException &e) {
        AddLogf(Error, "Error while processing file %s : %s", name.c_str(), e.what());
        throw e;
    }
}

std::string Preprocessor::GetOutput() {
    std::stringstream ss;

    ss << "// preprocessed shader code" << std::endl;
    ss << std::endl;
    ss << fmt::format("#version {}", version) << std::endl;
    ss << std::endl;

    ss << "// defines" << std::endl;
    for (auto &it : defines) {
        ss << fmt::format("#define {} {}", it.first, it.second) << std::endl;
    }

    ss << std::endl;
    ss << "// preprocesed code" << std::endl;

    for (auto line : output_buffer) {
        ss << line << std::endl;
    }
    return ss.str();
}

void Preprocessor::ClearOutput() {
    included_files.clear();
    output_buffer.clear();
}

void Preprocessor::Clear() {
    ClearOutput();
    defines.clear();
}

void Preprocessor::Process(const std::string &file_name, int level) {
    if (included_files.contains(file_name)) {
        output_buffer.pushf("//@ skip[%d] %s - already included", level, file_name.c_str());
        return;
    }

    included_files.insert(file_name);

    const iSharderPreprocesorCache::ReadBuffer *lines = nullptr;
    try {
        file_cache->ReadFile(file_name, lines);
    } catch (const std::exception &e) {
        AddLogf(Error, "Unable to load file '%s'", file_name.c_str());
        throw MissingFileException{file_name, level};
    }

    output_buffer.pushf("//@ start[%d] %s", level, file_name.c_str());

    unsigned line_index = 0;
    try {
        output_buffer.push(fmt::format("#line {}", 1));
        for (const auto &line : *lines) {
            ++line_index;
            bool handled = false;
            std::smatch match;
            for (const auto &token : kKnownTokens) {
                if (std::regex_search(line, match, token.regExp)) {
                    output_buffer.pushs("//@ %s", line);
                    (this->*token.handler)(file_name, line, level, std::move(match));
                    output_buffer.push(fmt::format("#line {}", line_index));
                    handled = true;
                    break;
                }
            }

            if (!handled) {
                output_buffer.push_back(line);
            }
        }
    } catch (ParseException &e) {
        AddLog(Error, fmt::format("Error while processing included file {} at line {}", file_name,
                                  line_index));
        throw e;
    }

    output_buffer.pushf("//@ end[%d] %s", level, file_name.c_str());
}

void Preprocessor::GenerateDefines() {
}

void Preprocessor::HandleIncludeToken(const std::string &file_name, const std::string &line,
                                      int level, std::smatch match) {
    std::string name = match[1];
    try {
        output_buffer.pushf("//@ pause[%d] %s", level, file_name.c_str());
        Process(name, level + 1);
        output_buffer.pushf("//@ continue[%d] %s", level, file_name.c_str());
    } catch (ParseException &) {
        AddLog(Error, fmt::format("Error while processing included file {}", file_name));
        throw;
    }
}

} // namespace MoonGlare::Renderer::Resources::Shader
