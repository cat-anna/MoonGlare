/*
 * Shader.h
 *
 *  Created on: 10-11-2013
 *      Author: Paweu
 */

#ifndef CSHADER_H_
#define CSHADER_H_

#include <Source/Renderer/Commands/CommandQueue.h>
#include <Source/Renderer/Commands/OpenGL/ShaderCommands.h>

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
		CameraMatrix,
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
	void SetCameraMatrix(const glm::mat4 & mat) { glUniformMatrix4fv(Location(ShaderParameters::CameraMatrix), 1, GL_FALSE, &mat[0][0]); }
	void SetCameraMatrix(const emath::fmat4 & mat) { glUniformMatrix4fv(Location(ShaderParameters::CameraMatrix), 1, GL_FALSE, (float*)&mat); }

	void SetCameraPos(const glm::vec3 & pos) { glUniform3fv(Location(ShaderParameters::CameraPos), 1, &pos[0]); }
	void SetCameraPos(const emath::fvec3 & pos) { glUniform3fv(Location(ShaderParameters::CameraPos), 1, &pos[0]); }

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


	void Bind(Renderer::Commands::CommandQueue &Queue, Renderer::Commands::CommandKey key) {
		Queue.PushCommand<Renderer::Commands::ShaderBind>(key)->m_Shader = Handle();
	}
	void Bind(Renderer::Commands::CommandQueue &Queue) {
		Renderer::Commands::CommandKey key{ 0 };
		Queue.PushCommand<Renderer::Commands::ShaderBind>(key)->m_Shader = Handle();
	}
	void SetModelMatrix(Renderer::Commands::CommandQueue &Queue, Renderer::Commands::CommandKey key, const emath::fmat4 &ModelMat) {
		auto loc = Location(ShaderParameters::ModelMatrix);
		if (!IsValidLocation(loc))
			return;

		auto arg = Queue.PushCommand<Renderer::Commands::ShaderSetUniformMatrix4>(key);
		arg->m_Location = loc;
		arg->m_Matrix = ModelMat;
	}

	void SetCameraMatrix(Renderer::Commands::CommandQueue &Queue, Renderer::Commands::CommandKey key, const emath::fmat4 &CameraMat) {
		auto loc = Location(ShaderParameters::CameraMatrix);
		if (!IsValidLocation(loc))
			return;

		auto arg = Queue.PushCommand<Renderer::Commands::ShaderSetUniformMatrix4>(key);
		arg->m_Location = loc;
		arg->m_Matrix = CameraMat;
	}

	void SetWorldMatrix(Renderer::Commands::CommandQueue &Queue, Renderer::Commands::CommandKey key, const emath::fmat4 & ModelMat, const emath::fmat4 &CameraMat) {
		auto loc = Location(ShaderParameters::WorldMatrix);
		if (!IsValidLocation(loc))
			return;

		auto arg = Queue.PushCommand<Renderer::Commands::ShaderSetUniformMatrix4>(key);
		arg->m_Location = loc;
		arg->m_Matrix = CameraMat * ModelMat;
	}
	void SetColor(Renderer::Commands::CommandQueue &Queue, Renderer::Commands::CommandKey key, const math::vec4 &color) {
		auto loc = Location(ShaderParameters::Material_BackColor);
		if (!IsValidLocation(loc))
			return;

		auto arg = Queue.PushCommand<Renderer::Commands::ShaderSetUniformVec3>(key);
		arg->m_Location = loc;
		arg->m_Vec = math::fvec3(color);
	}
	void SetColor(Renderer::Commands::CommandQueue &Queue, Renderer::Commands::CommandKey key, const emath::fvec4 &color) {
		auto loc = Location(ShaderParameters::Material_BackColor);
		if (!IsValidLocation(loc))
			return;

		auto arg = Queue.PushCommand<Renderer::Commands::ShaderSetUniformVec3>(key);
		arg->m_Location = loc;
		arg->m_Vec = math::fvec3(color[0], color[1], color[2]);
	}
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
