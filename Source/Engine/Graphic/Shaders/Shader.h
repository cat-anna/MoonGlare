/*
 * Shader.h
 *
 *  Created on: 10-11-2013
 *      Author: Paweu
 */

#ifndef CSHADER_H_
#define CSHADER_H_

namespace Graphic {
namespace Shaders {

class Shader : public NamedObject {
	SPACERTTI_DECLARE_CLASS_NOCREATOR(Shader, NamedObject);
public:
	Shader(GLuint ShaderProgram, const string &ProgramName);
	virtual ~Shader();

	enum class ShaderParameters : unsigned {
		WorldMatrix,
		ModelMatrix,
		CameraPos,

		Material_BackColor,
		Material_AlphaThreshold,

		Texture0,

		MaxValue,
	};

	static bool IsValidLocation(GLint location) { return location >= 0; }

	template<class T>
	void Bind(T *t) {
		T::Bind(this, t);
	}

	void Bind() { glUseProgram(m_ShaderProgram); }

	void SetWorldMatrix(const glm::mat4 & mat) { glUniformMatrix4fv(Location(ShaderParameters::WorldMatrix), 1, GL_FALSE, &mat[0][0]); }
	void SetModelMatrix(const glm::mat4 & mat) { glUniformMatrix4fv(Location(ShaderParameters::ModelMatrix), 1, GL_FALSE, &mat[0][0]); }
	void SetCameraPos(const glm::vec3 & pos) { glUniform3fv(Location(ShaderParameters::CameraPos), 1, &pos[0]); }

	void SetBackColor(const math::vec3 &color) { glUniform3fv(Location(ShaderParameters::Material_BackColor), 1, &color[0]); }
	void SetAlphaThreshold(float f) { glUniform1f(Location(ShaderParameters::Material_AlphaThreshold), f); }

	//void SetGamma(float val);

	void CheckStandardParamsLocation();
	GLuint Handle() const { return m_ShaderProgram; }
	GLint Location(ShaderParameters which) const { return m_ShaderParameters[(unsigned)which]; }
	GLint Location(const char* Name) const;

	struct ShaderParamNames : public std::vector <const char*> {
		ShaderParamNames();
	};
	static ShaderParamNames ParamNames;
private:
	GLint m_ShaderParameters[(unsigned)ShaderParameters::MaxValue];
	GLuint m_ShaderProgram;

	//GLuint m_GammaEnabled;
	//GLuint m_GammaInverted;
	//GLuint m_Gamma;
};

} // namespace Shaders 
} // namespace Graphic
#endif // CSHADER_H_ 
