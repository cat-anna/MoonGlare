#ifndef NFGRAPHIC_SHADERS_H_
#define NFGRAPHIC_SHADERS_H_

namespace Graphic {
namespace Shaders {
	class Shader;
	class ShaderManager;
	
	class ShadowMapShader;

	using ShaderClassRegister = GabiLib::DynamicClassRegister < Shader, int, const string&> ;
}// namespace Shaders

using Shader = Shaders::Shader;
using ShaderManager = Shaders::ShaderManager;

}// namespace Graphic

#endif // NFGRAPHIC_SHADERS_H_ 
