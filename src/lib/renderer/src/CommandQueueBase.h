#pragma once

#if 0

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

    static constexpr CommandKey Max() {
        return CommandKey { 0xFFFF };
    }
};
static_assert(sizeof(CommandKey) == sizeof(CommandKey::m_UIntValue), "CommandKey has invalid size!");

union CommandKeyEx {
    struct {
        uint32_t shaderIndex : 5;    //?
        uint32_t materialIndex : 12;
        uint32_t meshIndex : 12;
        uint32_t depth : 12;
    };
    uint64_t uintValue;
};
static_assert(sizeof(CommandKeyEx) == sizeof(uint64_t));
static_assert(std::is_pod_v<CommandKeyEx>);

} //namespace MoonGlare::Renderer::Commands

#if 0