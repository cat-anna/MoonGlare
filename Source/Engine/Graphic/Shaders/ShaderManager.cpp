/*
 * ShaderManager.cpp
 *
 *  Created on: 10-11-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>
#include "ShaderCode.h"			

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

struct ShaderTypeNames {
	ShaderTypeNames() {
#define __set(NAME) m_names[(unsigned)ShaderManager::ShaderType::NAME] = #NAME
		__set(glsl);
		__set(glfx);
		__set(glfx_root);
		__set(Invalid);
		__set(Unknown);
#undef __set
	}

	const char* operator[](ShaderManager::ShaderType type) const { return m_names[(unsigned)type]; }
private:
	const char* m_names[(unsigned)ShaderManager::ShaderType::MaxValue];
};

void ShaderManager::DumpShaders(std::ostream &out) {
	ShaderTypeNames names;
	out << "Shaders:\n";
	for (auto &it : m_Shaders) {
		auto &sd = it.second;
		char buf[128];
		sprintf(buf, "%40s [type: %s]\n",
				sd.Name.c_str(), names[sd.Type]);
		out << buf;
	}
	out << "\n";
}

#endif

//-------------------------------------------------------------------

Shader* ShaderManager::LoadShader(ShaderDefinition &sd, const string &ShaderName, ShaderCreateFunc CreateFunc, const string& Class) {
	if (sd.Type == ShaderType::Invalid) {
		AddLogf(Error, "Unable to load invalid shader '%s'", ShaderName.c_str());
		return nullptr;
	}

	const char *Name = ShaderName.c_str();
	const char *dot = strchr(Name, '.');
	unsigned len;
	//get name of shader without submode
	if (dot)
		len = dot - Name;
	else
		len = ShaderName.length();

	string OnlyName(Name, len);
	bool DoLoadDef = true;

	ShaderDefinition *parentsd = nullptr;

	if (dot) {
		//we are loading submode of shader. Look for parent
		auto it = m_Shaders.find(OnlyName);
		if (it == m_Shaders.end()) {
			//do nothing
		} else {
			parentsd = &it->second;
			if (parentsd->Type != ShaderType::glfx_root) {
				//Only glfx supports preloading.
				parentsd = nullptr;
			} else {
				//parrent definition is loaded
				DoLoadDef = false;
				sd.Type = ShaderType::glfx;
			}
		}
	} 
	
	if (DoLoadDef) {
		//Look for shader xml
		FileSystem::XMLFile xml;
		xml_node root;

		if (!GetFileSystem()->OpenResourceXML(xml, OnlyName, DataPath::Shaders)) {
			AddLogf(Warning, "There is no xml definition for shader shader named '%s' - using direct loader", OnlyName.c_str());
			sd.Type = ShaderType::glsl;
		} else {
			root = xml->document_element();
			const char *type = root.attribute("Type").as_string(ERROR_STR);

			sd.Type = ShaderType::Invalid;
			if (!strcmpi("glfx", type)) {
				sd.Type = ShaderType::glfx;
			} else if (!strcmpi("glsl", type)) {
				sd.Type = ShaderType::glsl;
			}

			switch (sd.Type) {
			case ShaderType::glfx:
			case ShaderType::glsl:
				break;
			default:
				AddLogf(Error, "Unknown type of shader '%s'", type);
				return nullptr;
			}
		}

		sd.Name = ShaderName;
		switch (sd.Type) {
		case ShaderType::glfx:
			parentsd = LoadShaderGlfx(sd, OnlyName, root);
			break;
		case ShaderType::glsl:
			parentsd = LoadShaderGlsl(sd, OnlyName, root);
			break;
		default:
			break;
		}

		if (!parentsd) {
			AddLog(Error, "An error has occur during loading shader definition");
			sd.Type = ShaderType::Invalid;
			return nullptr;
		}
	}

	string ProgramName;
	if (dot)
		ProgramName = dot + 1;
	else
		ProgramName = "Default";

	GLuint ShaderProg = 0;
	switch (sd.Type) {
	case ShaderType::glfx:
		ShaderProg = ConstructShaderGlfx(*parentsd, sd, ProgramName);
		break;
	case ShaderType::glsl:
		ShaderProg = sd.Handle;
		break;
	default:
		LogInvalidEnum(sd.Type);
		break;
	}

	if (!ShaderProg) {
		AddLogf(Error, "Unable to load shader '%s'", ShaderName.c_str());
		return 0;
	}

	Shader *s;
	if (CreateFunc) {
		s = CreateFunc(ShaderProg, ShaderName);
	} else {
		s = ShaderClassRegister::CreateClass(Class, ShaderProg, ShaderName);
	}
	if (!s) return nullptr;
	string NewName = Class;
	NewName += "_";
	NewName += ShaderName;
	sd.ShaderPtr = s;

	GetDataMgr()->NotifyResourcesChanged();

	return s;
}

//-------------------------------------------------------------------

ShaderManager::ShaderDefinition* ShaderManager::LoadShaderGlsl(ShaderDefinition &sd, const string &Name, const xml_node definition) {
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

	char typedefbuf[64] = "//shader type def\n";
	Lines[0] = "#version 420\n";
	Lines[1] = "//defines begin\n";
	Lines[2] = typedefbuf;
	Lines[3] = m_ShaderConfigurationDefs.c_str();
	//4
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

ShaderManager::ShaderDefinition* ShaderManager::LoadShaderGlfx(ShaderDefinition &sd, const string &Name, const xml_node definition) {
	const char* fname = definition.child("glfx").text().as_string(ERROR_STR);
	StarVFS::ByteTable data;
	if (!GetFileSystem()->OpenFile(fname, DataPath::Shaders, data)){
		AddLogf(Error, "Unable to load file for shader '%s'", sd.Name.c_str());
		return nullptr;
	}

	auto &parentsd = m_Shaders[Name];
	parentsd.Type = ShaderType::glfx_root;
	parentsd.Name = Name;
	sd.Parent = &parentsd;

	parentsd.Handle = glfxGenEffect();
	ShaderCodeVector CodeVec;
	CodeVec.push_copy((char*)data.get(), data.byte_size());
	PreproccesShaderCode(parentsd, CodeVec);
	if (!glfxParseEffectFromMemory(parentsd.Handle, CodeVec.Linear().c_str())) {
		AddLogf(Error, "Unable to parse shader file for shader '%s'", Name.c_str());
		AddLog(Error, "GLFX log: " << glfxGetEffectLog(parentsd.Handle));
		glfxDeleteEffect(parentsd.Handle);
		parentsd.Handle = 0;
		return 0;
	}
	return &parentsd;
}

GLuint ShaderManager::ConstructShaderGlfx(ShaderDefinition &parentsd, ShaderDefinition &sd, const string &Name) {
	int ShaderProg = glfxCompileProgram(parentsd.Handle, Name.c_str());
	if (ShaderProg == -1) {
		AddLogf(Error, "Unable to compile program '%s'", Name.c_str());
		AddLog(Error, "GLFX log: " << glfxGetEffectLog(parentsd.Handle));
		return 0;
	}
	return ShaderProg;
}

//-------------------------------------------------------------------

void ShaderManager::PreproccesShaderCode(ShaderDefinition &sd, ShaderCodeVector &CodeTable) {
	for (auto &it : CodeTable) {
		for (char *cit = it.get(); *cit;) {
			if (*cit != '#') {
				++cit;
				continue;
			}
			char *c_back = cit;
			++cit;

			if (strncmp(cit, "include", 7)) continue;
			char *file_begin = 0;
			char *file_end = 0;

			try {
				for (; *cit; ++cit) {
					if (file_begin && file_end) break;
					switch (*cit){
						case '<':
							file_begin = cit + 1;
							continue;
						case '>':
							file_end = cit;
							continue;
						case '"':
							if (!file_begin)file_begin = cit + 1;
							else file_end = cit;
							continue;
						case '\0': throw false;
						case '\n': throw 0u;
						default:
							continue;
					}
				}
			}
			catch (bool) {
				break;
			}
			catch (unsigned) {
			}

			string filename = string(file_begin, file_end - file_begin);

			for (; c_back <= file_end; ++c_back) *c_back = ' ';

			StarVFS::ByteTable data;

			if (!GetFileSystem()->OpenFile(filename, DataPath::Shaders, data)){
				AddLogf(Error, "Unable to load include file '%s' for shader '%s'", filename.c_str(), sd.Name.c_str());
				continue;
			}
			CodeTable.push_copy_front((char*)data.get(), data.byte_size());
			return PreproccesShaderCode(sd, CodeTable);
		}
	}
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

ShaderManager::ShaderDefinition::ShaderDefinition() : Name("?"), Handle(0), Type(ShaderType::Unknown), ShaderPtr(nullptr), Parent(nullptr) {}

ShaderManager::ShaderDefinition::~ShaderDefinition() { 
	delete ShaderPtr;
	ShaderPtr = nullptr;

	if (Handle != 0)
		switch (Type) {
		case ShaderType::glfx_root:
		case ShaderType::glfx:
				glfxDeleteEffect(Handle);
			break;
		case ShaderType::glsl:
			break;
		default:
			break;
	}
	Handle = 0;
}

} // namespace Shaders 
} // namespace Graphic
