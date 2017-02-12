#pragma once

#include "../CommandQueueBase.h"

namespace MoonGlare::Renderer::Commands{

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

} //namespace MoonGlare::Renderer::Commands
