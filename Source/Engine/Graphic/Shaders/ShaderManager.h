/*
 * ShaderManager.h
 *
 *  Created on: 10-11-2013
 *      Author: Paweu
 */
#ifndef CSHADERMANAGER_H_
#define CSHADERMANAGER_H_

namespace Graphic {
namespace Shaders {

class ShaderCodeVector;

class ShaderManager : public cRootClass {
	SPACERTTI_DECLARE_CLASS_SINGLETON(ShaderManager, cRootClass);
public:
	ShaderManager(Asset::AssetManager *AssetManager);
	virtual ~ShaderManager();

	bool Initialize();
	bool Finalize();

	Shader *GetShader(const string &Name) {
		return GetShader(Name, Shader::GetStaticTypeInfo()->GetName());
	} 

	using ShaderCreateFunc = Shader*(*)(GLuint ShaderProgram, const std::string &ProgramName);

	Shader *GetShader(const string &Name, const char* ClassName, ShaderCreateFunc CreateFunc = nullptr) {
		auto &shdef = m_Shaders[Name];
		if (!shdef.ShaderPtr)
			return LoadShader(shdef, Name, CreateFunc, ClassName);
		return shdef.ShaderPtr;
	} 

	template<class T>
	T* GetSpecialShader(const string &Name) {
		return GetShader(Name, T::GetStaticTypeInfo()->GetName());
	}

	template<class T>
	bool GetSpecialShader(const string &Name, T *&t) {
		t = static_cast<T*>(GetShader(Name, T::GetStaticTypeInfo()->GetName()));
		return t != nullptr;
	}

	template<class T>
	bool GetSpecialShaderType(const string &Name, T *&t) {
		struct S {
			static Shader* f(GLuint ShaderProgram, const std::string &ProgramName) {
				return new T(ShaderProgram, ProgramName);
			}
		};
		t = static_cast<T*>(GetShader(Name, "", &S::f));
		return t != nullptr;
	}

	struct Flags {
		enum {
			Ready	= 0x01,
		};
	};

	void DumpShaders(std::ostream &out);
private:
	struct ShaderDefinition {
		std::string Name;
		int Handle;
		Shader* ShaderPtr;

		ShaderDefinition& operator=(const ShaderDefinition&) = delete;
		ShaderDefinition(const ShaderDefinition&) = delete;
		ShaderDefinition(ShaderDefinition&&) = delete;

		ShaderDefinition();
		~ShaderDefinition();
	};
	
	Asset::Shader::Loader *m_ShaderLoader;
	std::string m_ShaderConfigurationDefs;
	std::unordered_map<string, ShaderDefinition> m_Shaders;

	void GenerateShaderConfiguration();

	Shader* LoadShader(ShaderDefinition &sd, const string &Name, ShaderCreateFunc CreateFunc, const string& Class);
	ShaderDefinition* LoadShaderGlsl(ShaderDefinition &sd, const string &Name);
};

} // namespace Shaders 
} // namespace Graphic 
#endif // CSHADERMANAGER_H_ 
