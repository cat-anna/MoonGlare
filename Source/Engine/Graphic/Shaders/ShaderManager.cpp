/*
 * ShaderManager.cpp
 *
 *  Created on: 10-11-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>

#include <Assets/AssetManager.h>
#include <Assets/Shader/Loader.h>

namespace Graphic {
namespace Shaders {

using ShaderCreateFunc = ShaderManager::ShaderCreateFunc;

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(ShaderManager);

ShaderManager::ShaderManager(Asset::AssetManager *AssetManager):
		cRootClass() {
	MoonGlareAssert(AssetManager);

	SetThisAsInstance();

	m_ShaderLoader = AssetManager->GetShaderLoader();
}

ShaderManager::~ShaderManager() {
	Finalize();
}

bool ShaderManager::Initialize() {
	GenerateShaderConfiguration();
	return true;
}

bool ShaderManager::Finalize() {
	m_Shaders.clear();
	return true;
}

//-------------------------------------------------------------------

#ifdef DEBUG_DUMP

void ShaderManager::DumpShaders(std::ostream &out) {
	out << "Shaders:\n";
	for (auto &it : m_Shaders) {
		auto &sd = it.second;
		char buf[128];
		sprintf(buf, "%40s\n",
				sd.Name.c_str());
		out << buf;
	}
	out << "\n";
}

#endif

//-------------------------------------------------------------------

Shader* ShaderManager::LoadShader(ShaderDefinition &sd, const string &ShaderName, ShaderCreateFunc CreateFunc, const string& Class) {

	sd.Name = ShaderName;
	if (!LoadShaderGlsl(sd, ShaderName)) {
		AddLog(Error, "An error has occur during loading shader definition");
		return nullptr;
	}

	GLuint ShaderProg = sd.Handle;
	if (!ShaderProg) {
		AddLogf(Error, "Unable to load shader '%s'", ShaderName.c_str());
		return nullptr;
	}

	Shader *s;
	if (CreateFunc) {
		s = CreateFunc(ShaderProg, ShaderName);
	} else {
		s = ShaderClassRegister::CreateClass(Class, ShaderProg, ShaderName);
	}
	if (!s) {
		GetRenderDevice()->RequestContextManip([ShaderProg] () {
			glDeleteProgram(ShaderProg);
		});
		return nullptr; //this leak loaded shader to opengl
	}

	string NewName = Class;
	NewName += "_";
	NewName += ShaderName;
	sd.ShaderPtr = s;

	GetDataMgr()->NotifyResourcesChanged();

	return s;
}

//-------------------------------------------------------------------

ShaderManager::ShaderDefinition* ShaderManager::LoadShaderGlsl(ShaderDefinition &sd, const string &Name) {
	using Asset::Shader::ShaderType;
	struct ShaderTypeInfo {
		ShaderType m_Type;
		GLuint m_GLID;
		const char *m_Name;
	};
	static const std::array<ShaderTypeInfo, static_cast<size_t>(ShaderType::MaxValue)> ShaderTypes = {
		ShaderTypeInfo{ ShaderType::Vertex, GL_VERTEX_SHADER, "vertex", },
		ShaderTypeInfo{ ShaderType::Fragment, GL_FRAGMENT_SHADER, "fragment", },
		ShaderTypeInfo{ ShaderType::Geometry, GL_GEOMETRY_SHADER, "geometry", },
	};

	Asset::Shader::ShaderCode code;
	if (!m_ShaderLoader->LoadCode(Name, code)) {
		AddLogf(Error, "Unable to load code for shader %s", Name.c_str());
		return nullptr;
	}

	static constexpr size_t MaxShaderLines = 8;
	using ShaderCodeBuffer = std::array<const char *, MaxShaderLines>;

	ShaderCodeBuffer Lines;
	Lines.fill("\n");

	char maindefbuf[64] = "//shader main def\n";
	char typedefbuf[64] = "//shader type def\n";
	Lines[0] = "#version 420\n";
	Lines[1] = "//defines begin\n";
	Lines[2] = typedefbuf;
	Lines[3] = typedefbuf;
	Lines[4] = m_ShaderConfigurationDefs.c_str();
	//5
	Lines[6] = "//preprocesed code begin\n";

	std::array<GLuint, static_cast<size_t>(ShaderType::MaxValue)> LoadedShaders;
	LoadedShaders.fill(0);
	unsigned LoadedCount = 0;
	auto DeleteShaders = [&LoadedCount, &LoadedShaders] {
		if (LoadedCount > 0) {
			for (auto i : LoadedShaders)
				glDeleteShader(i);
		}
	};

	bool Success = true;

	for (auto &shadertype : ShaderTypes) {
		auto index = static_cast<unsigned>(shadertype.m_Type);

		MoonGlareAssert(code.m_Code.size() > index);
		MoonGlareAssert(LoadedShaders.size() > index);

		if (code.m_Code[index].empty())
			continue;

		Lines.back() = code.m_Code[index].c_str();
		sprintf_s(typedefbuf, "#define %s_main main\n", shadertype.m_Name);
		sprintf_s(maindefbuf, "#define shader_%s\n", shadertype.m_Name);

		GLuint shader = glCreateShader(shadertype.m_GLID);
		LoadedShaders[index] = shader;

#ifdef DEBUG_DUMP
		{
			auto fname = Name + "." + shadertype.m_Name + ".glsl";
			std::replace(fname.begin(), fname.end(), '/', '.');
			std::ofstream of("logs/" + fname, std::ios::out | std::ios::binary);
			for (auto line : Lines)
				of << line;
			of.close();
		}
#endif

		glShaderSource(shader, Lines.size(), (const GLchar**)&Lines[0], NULL);	//TODO: check what is last argument
		glCompileShader(shader); //TODO: check what is return type

		GLint Result = GL_FALSE;
		int InfoLogLength = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
		
		if (InfoLogLength <= 0) {
			++LoadedCount;
			continue; //compiled ok
		}

		Success = false;

		std::string ShaderErrorMessage(InfoLogLength + 1, '\0');
		glGetShaderInfoLog(shader, InfoLogLength, NULL, &ShaderErrorMessage[0]);
		AddLogf(Error, "Unable to compile %s shader for %s. Error message:\n%s", shadertype.m_Name, Name.c_str(), ShaderErrorMessage.c_str());

		break;
	}

	if (!Success) {
		AddLogf(Error, "Shader compilation failed!");
		DeleteShaders();
		return nullptr;
	}

	//attach all shaders
	GLuint ProgramID = glCreateProgram();

	for(auto i : LoadedShaders) 
		if(i != 0)
			glAttachShader(ProgramID, i);

	//link program
	glLinkProgram(ProgramID);
	
	//check program for errors
	GLint Result = GL_FALSE;
	int InfoLogLength = 0;

	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);

	if(InfoLogLength > 1) {
		std::string ProgramErrorMessage(InfoLogLength+1, '\0');
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		AddLogf(Error, "Unable to link %s program. Error message:\n%s", Name.c_str(),  &ProgramErrorMessage[0]);
		AddLogf(Error, "Shader linking failed!");
		glDeleteProgram(ProgramID);
		DeleteShaders();
		return nullptr;
	}

	DeleteShaders();

	sd.Handle = ProgramID;
	return &sd;
}

//-------------------------------------------------------------------

void ShaderManager::GenerateShaderConfiguration() {
	m_ShaderConfigurationDefs.clear();
	
//	auto define = [this] (const char *id, std::string value) {
//		std::string def;
//		def = "#define ";
//		def += id;
//		def += " ";
//		def += value;
//		def += "\n";
//	
//	};

	//define("", )
}

//-------------------------------------------------------------------

ShaderManager::ShaderDefinition::ShaderDefinition() : Name("?"), Handle(-1), ShaderPtr(nullptr) {}

ShaderManager::ShaderDefinition::~ShaderDefinition() { 
	delete ShaderPtr;
	ShaderPtr = nullptr;
	
	auto handle = this->Handle;
	if (handle >= 0) {
		GetRenderDevice()->RequestContextManip([handle] () {
			glDeleteProgram(handle);
		});
	}
}

} // namespace Shaders 
} // namespace Graphic
