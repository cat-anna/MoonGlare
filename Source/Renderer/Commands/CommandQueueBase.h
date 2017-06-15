/*
* Generated by cppsrc.sh
* On 2016-09-21 19:16:53,84
* by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include "../Configuration.Renderer.h"

namespace MoonGlare::Renderer::Commands {

using CommandArgument = void*;
using CommandFunction = void(*)(CommandArgument);

template<typename CMDARG>
struct CommandTemplate {
	using Argument = CMDARG;
	static size_t ArgumentSize() {
		return sizeof(Argument);
	}
	static CommandFunction GetFunction() {
		return reinterpret_cast<CommandFunction>(&Argument::Execute);
	}
};

template<typename CMDARG>
struct RunnableCommandTemplate  {
	using Argument = CMDARG;
	static size_t ArgumentSize() {
		return sizeof(Argument);
	}

	static void Execute(CommandArgument* arg) {
		Argument *argptr = reinterpret_cast<Argument*>(arg);
		argptr->Run();
	}
	static CommandFunction GetFunction() {
		return reinterpret_cast<CommandFunction>(&Execute);
	}
};

template<typename HandleType>
struct ResourceCommandInfo;

template<typename HandleType>
struct ResourceBindCommandBase {
	using HandleArrayType = typename ResourceCommandInfo<HandleType>::HandleArrayType;
	HandleArrayType m_HandlePtr;
};

template<>
struct ResourceCommandInfo<TextureResourceHandle> {
	using HandleArrayType = Device::TextureHandle*;
	using HandleType = TextureResourceHandle;
};

template<>
struct ResourceCommandInfo<VAOResourceHandle> {
	using HandleArrayType = GLuint*;
};

using TextureCommandBase = ResourceBindCommandBase<TextureResourceHandle>;
using VAOBindCommandBase = ResourceBindCommandBase<VAOResourceHandle>;

union CommandKey {
	uint16_t m_UIntValue;
	struct {
		//	uint32_t m_Ptr;
		uint16_t m_Order;
	};
};

static_assert(sizeof(CommandKey) == sizeof(CommandKey::m_UIntValue), "CommandKey has invalid size!");

} //namespace MoonGlare::Renderer::Commands 
