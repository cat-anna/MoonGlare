#pragma once

#include "../CommandQueueBase.h"

#include "../../Material.h"

namespace MoonGlare::Renderer::Commands {

struct ShaderBindArgument {
	Device::ShaderHandle m_Shader;
	static void Execute(const ShaderBindArgument *arg) {
		glUseProgram(arg->m_Shader);
	}
};
using ShaderBind = CommandTemplate<ShaderBindArgument>;

//---------------------------------------------------------------------------------------

struct ShaderSetUniformMatrix4Argument {
	Device::ShaderUniformHandle m_Location;
	math::RawMat4 m_Matrix;
	static void Execute(const ShaderSetUniformMatrix4Argument *arg) {
		glUniformMatrix4fv(arg->m_Location, 1, GL_FALSE, (const float*)&arg->m_Matrix);
	}
};
using ShaderSetUniformMatrix4 = CommandTemplate<ShaderSetUniformMatrix4Argument>;

struct ShaderSetUniformVec4Argument {
	Device::ShaderUniformHandle m_Location;
	math::RawVec4 m_Vec;
	static void Execute(const ShaderSetUniformVec4Argument *arg) {
		glUniform4fv(arg->m_Location, 1, &arg->m_Vec[0]);
	}
};
using ShaderSetUniformVec4 = CommandTemplate<ShaderSetUniformVec4Argument>;

struct ShaderSetUniformVec3Argument {
	Device::ShaderUniformHandle m_Location;
	math::RawVec3 m_Vec;
	static void Execute(const ShaderSetUniformVec3Argument *arg) {
		glUniform3fv(arg->m_Location, 1, &arg->m_Vec[0]);
	}
};
using ShaderSetUniformVec3 = CommandTemplate<ShaderSetUniformVec3Argument>;

struct ShaderSetUniformVec2Argument {
	Device::ShaderUniformHandle m_Location;
	float m_Vec[2];
	static void Execute(const ShaderSetUniformVec2Argument *arg) {
		glUniform2fv(arg->m_Location, 1, arg->m_Vec);
	}
};
using ShaderSetUniformVec2 = CommandTemplate<ShaderSetUniformVec2Argument>;

struct ShaderSetUniformIVec2Argument {
	Device::ShaderUniformHandle m_Location;
	int m_Vec[2];
	static void Execute(const ShaderSetUniformIVec2Argument *arg) {
		glUniform2iv(arg->m_Location, 1, arg->m_Vec);
	}
};
using ShaderSetUniformIVec2 = CommandTemplate<ShaderSetUniformIVec2Argument>;

struct ShaderSetUniformFloatArgument {
	Device::ShaderUniformHandle m_Location;
	float m_Float;
	static void Execute(const ShaderSetUniformFloatArgument *arg) {
		glUniform1f(arg->m_Location, arg->m_Float);
	}
};
using ShaderSetUniformFloat = CommandTemplate<ShaderSetUniformFloatArgument>;

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

namespace detail {

struct ShaderResourceBindArgument {
	Device::ShaderHandle *m_Shader;
	static void Execute(const ShaderResourceBindArgument *arg) {
		glUseProgram(*arg->m_Shader);
	}
};

struct ShaderResourcSetUniformArgumentBase {
	static void Set(Device::ShaderUniformHandle h, const emath::fmat4 &matrix) {
		glUniformMatrix4fv(h, 1, GL_FALSE, (const float*)&matrix);
	}

	static void Set(Device::ShaderUniformHandle h, const emath::fvec4 &v) {
		glUniform4fv(h, 1, (const float*)&v);
	}
	static void Set(Device::ShaderUniformHandle h, const emath::fvec3 &v) {
		glUniform3fv(h, 1, (const float*)&v);
	}
	static void Set(Device::ShaderUniformHandle h, const emath::fvec2 &v) {
		glUniform2fv(h, 1, (const float*)&v);
	}


	static void Set(Device::ShaderUniformHandle h, const emath::ivec4 &v) {
		glUniform4iv(h, 1, (const int*)&v);
	}
	static void Set(Device::ShaderUniformHandle h, const emath::ivec3 &v) {
		glUniform3iv(h, 1, (const int*)&v);
	}
	static void Set(Device::ShaderUniformHandle h, const emath::ivec2 &v) {
		glUniform2iv(h, 1, (const int*)&v);
	}

	static void Set(Device::ShaderUniformHandle h, float v) {
		glUniform1f(h, v);
	}
	static void Set(Device::ShaderUniformHandle h, int v) {
		glUniform1i(h, v);
	}
};

template<typename T>
struct ShaderResourcSetUniformArgument {
	Device::ShaderUniformHandle* m_Handle;
	T m_Value;
	static void Execute(const ShaderResourcSetUniformArgument *arg) {
		ShaderResourcSetUniformArgumentBase::Set(*arg->m_Handle, arg->m_Value);
	}
};

template<typename T>
struct ShaderResourcSetNamedUniformArgument {
    Device::ShaderHandle *shaderHandle;
    T value;
    const char *name;
    static void Execute(const ShaderResourcSetNamedUniformArgument *arg) {            
        auto loc = glGetUniformLocation(*arg->shaderHandle, arg->name);
        ShaderResourcSetUniformArgumentBase::Set(loc, arg->value);
    }
};

}
using ShaderResourceBind = CommandTemplate<detail::ShaderResourceBindArgument>;

template<typename T>
using ShaderResourcSetUniform = CommandTemplate<detail::ShaderResourcSetUniformArgument<T>>;

template<typename T>
using ShaderResourcSetNamedUniform = CommandTemplate<detail::ShaderResourcSetNamedUniformArgument<T>>;

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

namespace detail {

struct ShaderBindMaterialResourceArgument {
	using Conf = Configuration::Shader;

    emath::fvec3 diffuseColor;
    emath::fvec3 specularColor;
    emath::fvec3 emissiveColor;

    float shinessExponent; 

    Device::ShaderUniformHandle diffuseColorLocation;
    Device::ShaderUniformHandle specularColorLocation;
    Device::ShaderUniformHandle emissiveColorLocation;

    Device::ShaderUniformHandle shinessExponentLocation;
    Device::ShaderUniformHandle useNormalMapLocation;

    Material::Array<uint8_t> mapUnit;
    Material::Array< Device::TextureHandle> mapHandle;

    using SET = ShaderResourcSetUniformArgumentBase;

    struct BIND {
        static void Set(uint8_t unitIndex, Device::TextureHandle h) {
            glActiveTexture(GL_TEXTURE0 + unitIndex);
            glBindTexture(GL_TEXTURE_2D, h);
        }
    };
	void Run() const {
        SET::Set(diffuseColorLocation, diffuseColor);
        SET::Set(specularColorLocation, specularColor);
        SET::Set(emissiveColorLocation, emissiveColor);

        SET::Set(shinessExponentLocation, shinessExponent);
        SET::Set(useNormalMapLocation, mapHandle[(size_t)Material::MapType::Normal] != Device::InvalidTextureHandle);

        for (size_t i = 0; i < mapHandle.size(); ++i) {
            BIND::Set(mapUnit[i], mapHandle[i]);
        }
	}

	static void Execute(const ShaderBindMaterialResourceArgument *arg) {
		arg->Run();
	}
};

}

using ShaderBindMaterialResource = CommandTemplate<detail::ShaderBindMaterialResourceArgument>;


} //namespace MoonGlare::Renderer::Commands
