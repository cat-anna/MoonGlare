#include <pch.h>
#include "../Graphic.h"
//#include "EngineHeader.h"

namespace Graphic {
namespace Dereferred {

SPACERTTI_IMPLEMENT_ABSTRACT_CLASS(LightingPassShader);

LightingPassShader::LightingPassShader(GLuint ShaderProgram, const string &ProgramName):
		BaseClass(ShaderProgram, ProgramName) {

	glUniform1i(Location("PositionMap"),		SamplerIndex::Position);
    glUniform1i(Location("ColorMap"),			SamplerIndex::Diffuse);
    glUniform1i(Location("NormalMap"),			SamplerIndex::Normal);

    glUniform1i(Location("PlaneShadowMap"),		SamplerIndex::PlaneShadow);

	m_ShadowMapSizeLocation		= Location("ShadowMapSize");
	m_LightMatrixLocation		= Location("LightMatrix"); 
	m_EnableShadowsLocation		= Location("EnableShadowTest");  
}      
       
LightingPassShader::~LightingPassShader() {
}  
  
} //namespace Dereferred
} //namespace Graphic 
    