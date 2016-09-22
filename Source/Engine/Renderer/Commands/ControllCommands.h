#pragma once
#ifndef GLCONTROLLCOMMANDS_H_
#define GLCONTROLLCOMMANDS_H_

#include "../OpenGL/nfOpenGL.h"

namespace MoonGlare {
namespace Renderer {
namespace OpenGL {
namespace Commands {

struct EnableArgument {
	GLenum m_What;
	static void Execute(const EnableArgument *arg) {
		glEnable(arg->m_What);
	}
};
using Enable = CommandTemplate<EnableArgument>;

struct DisableArgument {
	GLenum m_What;
	static void Execute(const DisableArgument *arg) {
		glDisable(arg->m_What);
	}
};
using Disable = CommandTemplate<DisableArgument>;

} //namespace Commands
} //namespace OpenGL
} //namespace Renderer 
} //namespace MoonGlare 

#endif // NFRENDERER_H_
