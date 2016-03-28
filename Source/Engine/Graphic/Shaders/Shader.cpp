/*
 * Shader.cpp
 *
 *  Created on: 10-11-2013
 *      Author: Paweu
 */
#include <pch.h>
#include "../Graphic.h"
 
namespace Graphic {
namespace Shaders {

Shader::ShaderParamNames::ShaderParamNames() {
	resize((unsigned)ShaderParameters::MaxValue, "");
#define _add(N) at((unsigned)ShaderParameters::N) = #N
#define _addx(N, W) at((unsigned)ShaderParameters::N) = W
	_add(WorldMatrix);
	_add(ModelMatrix);

	_add(CameraPos);
	 
	_addx(Material_BackColor, "Material.BackColor");
	_addx(Material_AlphaThreshold, "Material.AlphaThreshold");

	_add(Texture0);
#undef _add
#undef _addx
}

GABI_IMPLEMENT_CLASS_NOCREATOR(Shader);
ShaderClassRegister::Register<Shader> ShaderReg;

Shader::ShaderParamNames Shader::ParamNames;

GLint Shader::Location(const char* Name) const { 
	GLint loc = glGetUniformLocation(Handle(), Name); 

#ifdef DEBUG
	static std::unordered_map<string, int> _ReportedErrorLocations;
	if (loc == -1) {
		string sloc;
		sloc.reserve(128);
		sloc += GetName();
		sloc += "_";
		sloc += Name;
		if (_ReportedErrorLocations.find(sloc) != _ReportedErrorLocations.end())
			return -1;
		_ReportedErrorLocations[sloc] = 1;
		AddLogf(Warning, "Unable to get location of parameter '%s' in shader '%s'", Name, GetName().c_str());
		return -1;
	}
#endif
	return loc;
}

void Shader::CheckStandardParamsLocation() {
	for (unsigned i = 0, j = (unsigned)ShaderParameters::MaxValue; i < j; ++i) {
		m_ShaderParameters[i] = Location(ParamNames[i]);
	}
}
#if 0
void Shader::SetGamma(float val) {
	//if (val < 0) {
		glUniform1i(m_Gamma, 0);
	//} else {
		glUniform1i(m_Gamma, 1);
		glUniform1f(m_GammaInverted, 1.0f/val);
		glUniform1f(m_Gamma, val);
	//}
}
#endif
Shader::Shader(GLuint ShaderProgram, const string &ProgramName):
			m_ShaderProgram(ShaderProgram) {
	Bind();
	SetName(ProgramName);
	CheckStandardParamsLocation(); 
	  
	auto ScreenSize = math::fvec2(GetRenderDevice()->GetContextSize());
	glUniform2fv(Location("ScreenSize"), 1, &ScreenSize[0]);
	 
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(Location(ShaderParameters::Texture0), 0);
	SetBackColor(math::fvec3(1, 1, 1));
	SetAlphaThreshold(0.1f);
	
	//m_GammaEnabled	   = Location("gGammaEnabled");
	//m_GammaInverted	   = Location("gGammaInverted");
	//m_Gamma			   = Location("gGamma");

	//SetGamma(-1.0f);

	StaticFog fog{ 0 };
	fog.Bind(this, &fog);
}  
 
Shader::~Shader() { 
	glDeleteProgram(m_ShaderProgram); 
}

} // namespace Shaders 
} // namespace Graphic 
