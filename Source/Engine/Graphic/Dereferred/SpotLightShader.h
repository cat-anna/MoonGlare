/*
	Generated by cppsrc.sh
	On 2015-01-19 22:21:25,72
	by Paweu
*/

#pragma once
#ifndef SpotLightShader_H
#define SpotLightShader_H

namespace Graphic {
namespace Dereferred {

class SpotLightShader : public LightingPassShader {
	SPACERTTI_DECLARE_CLASS_NOCREATOR(SpotLightShader, LightingPassShader);
public:
 	SpotLightShader(GLuint ShaderProgram, const string &ProgramName);
 	virtual ~SpotLightShader();

	void Bind(const ::MoonGlare::Renderer::Light::SpotLight &light) const {
		BaseClass::BindLightBase(light.m_Base);
		glUniform3fv(m_PositionLocation, 1, &light.m_Position[0]);
		glUniform3fv(m_DirectionLocation, 1, &light.m_Direction[0]);
		glUniform1f(m_CutOffLocation, light.m_CutOff);
		glUniform1f(m_AttenuationLinearLocation, light.m_Attenuation.m_Linear);
		glUniform1f(m_AttenuationExpLocation, light.m_Attenuation.m_Exp);
		glUniform1f(m_AttenuationConstantLocation, light.m_Attenuation.m_Constant);
		glUniform1f(m_AttenuationMinThresholdLocation, light.m_Attenuation.m_Threshold);
	}
protected:
	GLuint m_DirectionLocation;
	GLuint m_CutOffLocation;
	GLuint m_PositionLocation;
	GLuint m_AttenuationLinearLocation;
	GLuint m_AttenuationExpLocation;
	GLuint m_AttenuationConstantLocation;
	GLuint m_AttenuationMinThresholdLocation;
};

} //namespace Dereferred 
} //namespace Graphic 

#endif
