#pragma once

#include "../OpenGL/nfOpenGL.h"

namespace MoonGlare {
namespace Renderer {
namespace Commands {

struct Texture2DBindArgument {
	TextureHandle m_Texture;
	static void Execute(const Texture2DBindArgument *arg) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, arg->m_Texture);
	}
};
using Texture2DBind = CommandTemplate<Texture2DBindArgument>;

struct Texture2DBindUnitArgument {
	TextureHandle m_Texture;
	unsigned m_UnitIndex;
	static void Execute(const Texture2DBindUnitArgument *arg) {
		glActiveTexture(GL_TEXTURE0 + arg->m_UnitIndex);
		glBindTexture(GL_TEXTURE_2D, arg->m_Texture);
	}
};
using Texture2DBindUnit = CommandTemplate<Texture2DBindUnitArgument>;

} //namespace Commands
} //namespace Renderer 
} //namespace MoonGlare 
