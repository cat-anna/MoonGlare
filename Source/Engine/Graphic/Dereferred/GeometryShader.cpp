#include <pch.h>
#include "../Graphic.h"

namespace Graphic {
namespace Dereferred {

SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(GeometryPassShader);
Shaders::ShaderClassRegister::Register<GeometryPassShader> ShaderReg;

GeometryPassShader::GeometryPassShader(GLuint ShaderProgram, const string &ProgramName) : 
		BaseClass(ShaderProgram, ProgramName) {
}

GeometryPassShader::~GeometryPassShader() {
}

} //namespace Dereferred
} //namespace Graphic 
