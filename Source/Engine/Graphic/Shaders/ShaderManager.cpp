/*
 * ShaderManager.cpp
 *
 *  Created on: 10-11-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>
#include "ShaderCode.h"

namespace Graphic {
namespace Shaders {

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(ShaderManager);

ShaderManager::ShaderManager(): 
		cRootClass(), 
		m_Flags(0) {
	SetThisAsInstance();
}

ShaderManager::~ShaderManager() {
	Finalize();
}

bool ShaderManager::Initialize() {
	SetReady(true);
	return true;
}

bool ShaderManager::Finalize() {
	if (!IsReady()) return true;

	m_Shaders.clear();

	SetReady(false);
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

Shader* ShaderManager::LoadShader(ShaderDefinition &sd, const string &ShaderName, const string& Class) {
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
	
	if(DoLoadDef) {
		//Look for shader xml
		FileSystem::XMLFile xml;
		if (!GetFileSystem()->OpenResourceXML(xml, OnlyName, DataPath::Shaders)) {
			AddLogf(Error, "There is no shader named '%s'", OnlyName.c_str());
			sd.Type = ShaderType::Invalid;
			return nullptr;
		}

		const xml_node root = xml->document_element();
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
		ShaderProg = ConstructShaderGlsl(*parentsd, sd, ProgramName);
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
	s = ShaderClassRegister::CreateClass(Class, ShaderProg, ShaderName);
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
	std::vector<GLuint> LoadedShaders;

	struct ShaderType {
		const char * ext;
		GLuint value;
	};
	static const ShaderType ShaderTypes[] = {
		{ "vs", GL_VERTEX_SHADER},
		{ "fs", GL_FRAGMENT_SHADER},
		{ "gs", GL_GEOMETRY_SHADER},
		{ 0 }
	};

	try {
		for (const ShaderType *type = ShaderTypes; type->ext; ++type) {

			string FileName = sd.Name;
			FileName += ".";
			FileName += type->ext;
			StarVFS::ByteTable data;
			if (!GetFileSystem()->OpenFile(FileName, DataPath::Shaders, data)){
				AddLogf(Hint, "Unable to load file '%s' for shader '%s' ", FileName.c_str(), sd.Name.c_str());
				continue;
			}
			// Compile 
			ShaderCodeVector CodeVec;
			CodeVec.push_copy((char*)data.get(), data.byte_size());
			PreproccesShaderCode(sd, CodeVec);
			GLuint shader = glCreateShader(type->value);
			glShaderSource(shader, CodeVec.len(), (const GLchar**)CodeVec.get(), NULL);
			glCompileShader(shader);
			// Check Shader
			GLint Result = GL_FALSE;
			int InfoLogLength;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &Result);
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
			if(InfoLogLength > 1) {
				std::vector<char> ShaderErrorMessage(InfoLogLength+1);
				glGetShaderInfoLog(shader, InfoLogLength, NULL, &ShaderErrorMessage[0]);
				AddLogf(Error, "Unable to compile '%s' shader. Error message:\n%s", type->ext, &ShaderErrorMessage[0]);
				throw (unsigned)shader;
			}
			LoadedShaders.push_back(shader);
		}

		//attach all shaders
		GLuint ProgramID = glCreateProgram();
		for(auto &i : LoadedShaders) glAttachShader(ProgramID, i);
		//link program
		glLinkProgram(ProgramID);
		//check program for errors
		GLint Result = GL_FALSE;
		int InfoLogLength;
		glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if(InfoLogLength > 1) {
			std::vector<char> ProgramErrorMessage(InfoLogLength+1);
			glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			AddLogf(Error, "Unable to link %s program. Error message:\n%s", Name.c_str(),  &ProgramErrorMessage[0]);
			glDeleteProgram(ProgramID);
			throw false;
		}

		//delete all shders
		for(auto &i : LoadedShaders) glDeleteShader(i);
		sd.Handle = ProgramID;
		return &sd;
	} 
	catch(bool){ }//just exit
	catch(unsigned shader){//shader created, but not pushed into table, remove it
		glDeleteShader(shader);
	}
	catch (...) { } 
	for(auto &i : LoadedShaders) glDeleteShader(i);
	LoadedShaders.clear();
	return 0;
}

GLuint ShaderManager::ConstructShaderGlsl(ShaderDefinition &parentsd, ShaderDefinition &sd, const string &Name) {
	return sd.Handle;
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
