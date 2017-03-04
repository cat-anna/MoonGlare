/*
* Generated by cppsrc.sh
* On 2017-02-13 22:45:35,93
* by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include "../nfRenderer.h"
#include "../Configuration.Renderer.h"

#include "../Commands/CommandQueue.h"
#include "../Commands/OpenGL/ShaderCommands.h"
#include "../Commands/OpenGL/TextureCommands.h"

#include "MaterialManager.h"

namespace MoonGlare::Renderer::Resources {

template<typename Descriptor>
struct
	//alignas(16) 
	ShaderBuilder {
//	static_assert(std::is_trivial<ShaderBuilder>::value, "must be trivial!");
//	static_assert((sizeof(ShaderBuilder) % 16) == 0, "Invalid size!");

	using Conf = Configuration::Shader;
	using Uniform = typename Descriptor::Uniform;
	using Sampler = typename Descriptor::Sampler;

	void SelfTest() {
		RendererAssert(this);
		RendererAssert(m_UniformsPtr);
		RendererAssert(m_HandlePtr);
		RendererAssert(m_ResourceManager);
	}

	void Bind() {
		SelfTest();
		m_Queue->MakeCommand<Commands::ShaderResourceBind>(m_HandlePtr);
	}
	void UnBind() {
		SelfTest();
		m_Queue->MakeCommand<Commands::ShaderBind>(InvalidShaderHandle);
	}

	template<Uniform u, typename T>
	void Set(const T &t, Commands::CommandKey key = Commands::CommandKey()) {
		auto arg = m_Queue->PushCommand<Commands::ShaderResourcSetUniform<T>>(key);
		arg->m_Handle = &(*m_UniformsPtr)[static_cast<uint32_t>(u)];
		arg->m_Value = t;
	}

	template<Sampler SamplerUnit>
	void Set(TextureResourceHandle h, Commands::CommandKey key = Commands::CommandKey()) {
		auto texres = m_Queue->PushCommand<Renderer::Commands::Texture2DResourceBindUnit>(key);
		texres->m_Handle = h;
		texres->m_UnitIndex = static_cast<uint16_t>(SamplerUnit);
		texres->m_HandleArray = m_ResourceManager->GetTextureResource().GetHandleArrayBase();
	}

	void SetMaterial(MaterialResourceHandle h, Commands::CommandKey key = Commands::CommandKey()) {
		auto *mat = this->m_ResourceManager->GetMaterialManager()->GetMaterial(h);
		if (!mat) {
			//TODO: do sth
			return;
		}
		SetMaterial(*mat, key);
	}

	void SetMaterial(Material& mat, Commands::CommandKey key = Commands::CommandKey()) {
		Set<Sampler::DiffuseMap>(mat.m_DiffuseMap, key);
		Set<Uniform::DiffuseColor>(mat.m_DiffuseColor, key);
	}

	Commands::CommandQueue *m_Queue;
	Conf::UniformLocations *m_UniformsPtr;
	ShaderHandle* m_HandlePtr;
	ResourceManager* m_ResourceManager;
};

template<typename Descriptor>
struct
	//alignas(16) 
	ShaderExecutor {
	//	static_assert(std::is_trivial<ShaderBuilder>::value, "must be trivial!");
	//	static_assert((sizeof(ShaderBuilder) % 16) == 0, "Invalid size!");
	using Conf = Configuration::Shader;
	using Uniform = typename Descriptor::Uniform;
	using Sampler = typename Descriptor::Sampler;

	void SelfTest() {
		RendererAssert(this);
		RendererAssert(m_UniformsPtr);
		RendererAssert(m_HandlePtr);
		RendererAssert(m_ResourceManager);
	}

	void Bind() {
		SelfTest();
		Run<Commands::ShaderResourceBind>(m_HandlePtr);
	}
	void UnBind() {
		SelfTest();
		Run<Commands::ShaderBind>(InvalidShaderHandle);
	}

	template<Uniform u, typename T>
	void Set(const T &t, Commands::CommandKey key = Commands::CommandKey()) {
		SelfTest();
		(void)key;
		Run<Commands::ShaderResourcSetUniform<T>>(&(*m_UniformsPtr)[static_cast<uint32_t>(u)], t);
	}

	template<Sampler SamplerUnit>
	void Set(TextureResourceHandle h, Commands::CommandKey key = Commands::CommandKey()) {
		Renderer::Commands::Texture2DResourceBindUnit::Argument arg;
		arg.m_Handle = h;
		arg.m_HandleArray = m_ResourceManager->GetTextureResource().GetHandleArrayBase();
		arg.m_UnitIndex = static_cast<uint16_t>(SamplerUnit);
		Run<Renderer::Commands::Texture2DResourceBindUnit>(arg);
	}

	void SetMaterial(MaterialResourceHandle h, Commands::CommandKey key = Commands::CommandKey()) {
		auto *mat = this->m_ResourceManager->GetMaterialManager()->GetMaterial(h);
		if (!mat) {
			//TODO: do sth
			return;
		}
		SetMaterial(*mat, key);
	}

	void SetMaterial(Material& mat, Commands::CommandKey key = Commands::CommandKey()) {
		Set<Sampler::DiffuseMap>(mat.m_DiffuseMap, key);
		Set<Uniform::DiffuseColor>(mat.m_DiffuseColor, key);
	}

	Conf::UniformLocations *m_UniformsPtr;
	ShaderHandle* m_HandlePtr;
	ResourceManager* m_ResourceManager;
	//void* _padding1;
private:
	template<typename T, typename ... ARG>
	void Run(ARG && ...arg) {
		T::Argument a{ std::forward<ARG>(arg)... };
		a.Execute(&a);
	}
};

} //namespace MoonGlare::Renderer::Resources 
