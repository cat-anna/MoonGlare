/*
  * Generated by cppsrc.sh
  * On 2017-02-04 20:57:16,59
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#include "../AssetManager.h"
#include "../FileSystem.h"
#include "Loader.h"

#include "Preprocessor.h"

namespace MoonGlare::Asset::Shader {

Loader::Loader(FileSystem *fs):
	m_FileSystem(fs) {

	MoonGlareAssert(fs);

	m_FileCache = std::make_unique<FileCache>(fs);
}

Loader::~Loader() {
}

bool Loader::Initialize() {
	return true;
}

bool Loader::Finalize() {
	return true;
}

bool Loader::LoadCode(const std::string &Name, ShaderCode &Output) {
	struct ShaderFileInfo {
		ShaderType m_Type;
		const char *m_Ext;
		const char *m_Name;
	};

	static const std::array<ShaderFileInfo, static_cast<size_t>(ShaderType::MaxValue)> ShaderFiles = {
		ShaderFileInfo{ ShaderType::Vertex,   "vs", "vertex", },
		ShaderFileInfo{ ShaderType::Fragment, "fs", "fragment", },
		ShaderFileInfo{ ShaderType::Geometry, "gs", "geometry", },
	};

	unsigned LoadCount = 0;
	bool Success = true;

	m_FileCache->Clear();
	Preprocessor preproc(m_FileCache.get());

	auto TryLoad = [&preproc, &Name](std::string fn, const ShaderFileInfo& shaderfile) -> bool {
		preproc.ClearOutput();
		try {
			preproc.PushFile(fn);
		}
		catch (Preprocessor::ParseException &e) {
			if (e.m_IncludeLevel > 0) {
				AddLogf(Error, "Failure during preprocessing file %s for shader %s", fn.c_str(), Name.c_str());
				throw e;
			} else {
				// no code for sub-shader
				return false;
			}
		}
		return true;
	};

	for (auto &shaderfile : ShaderFiles) {
		try {
			if (!TryLoad(Name + "." + shaderfile.m_Ext, shaderfile)) {
				AddLogf(Warning, "No dedicated file for %s shader %s", shaderfile.m_Name, Name.c_str());
				if (!TryLoad(Name + ".glsl", shaderfile)) {
					AddLogf(Warning, "No master file for %s shader %s", shaderfile.m_Name, Name.c_str());
					continue;
				}
			}
		}
		catch (Preprocessor::ParseException &) {
			AddLogf(Error, "File for %s shader %s", shaderfile.m_Name, Name.c_str());
			Success = false;
			break;
		}

		std::string code;
		preproc.GetOutput(code);

		++LoadCount;

		auto index = static_cast<unsigned>(shaderfile.m_Type);
		Output.m_Code[index].swap(code);
	}

	if (Success)
		return true;

	Output.m_Code.fill(std::string());
	return false;
}

} //namespace MoonGlare::Asset::Shader
