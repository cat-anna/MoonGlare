#include <pch.h>
#include "../Graphic.h"

namespace Graphic {
namespace Dereferred {

Shaders::ShaderClassRegister::Register<GeometryPassShader> ShaderReg;
SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(GeometryPassShader);

GeometryPassShader::GeometryPassShader(GLuint ShaderProgram, const string &ProgramName) : 
		BaseClass(ShaderProgram, ProgramName) {
}

GeometryPassShader::~GeometryPassShader() {
}

} //namespace Dereferred
} //namespace Graphic 
