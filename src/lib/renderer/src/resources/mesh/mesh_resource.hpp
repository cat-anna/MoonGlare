#pragma once

// #include "../../Configuration.Renderer.h"
// #include "../../Configuration/Mesh.h"
// #include "../../nfRenderer.h"
// #include <Foundation/Resources/Mesh.h>

#include "async_loader.hpp"
#include "renderer/resources.hpp"
#include <memory>
#include <string>

namespace MoonGlare::Renderer::Resources {
// using namespace MoonGlare::Resources;

class MeshManager final : public iMeshManager {
    // using ThisClass = MeshManager;
    // using Conf = Configuration::Mesh;
    // using ConfRes = Configuration::Resources;
public:
    MeshManager(std::shared_ptr<iAsyncLoader> async_loader, std::shared_ptr<iContextResourceLoader> context_loader);
    ~MeshManager() override;

    // ResourceManager* GetResourceManager() { return resourceManager; }

    // MeshResourceHandle Allocate();
    // void Release(MeshResourceHandle hin);

    MeshResourceHandle LoadMesh(FileResourceId res_id) override;
    MeshResourceHandle CreateMesh(const MeshSource &source, FileResourceId res_id) override;

    // void ApplyMeshSource(MeshResourceHandle h, const MeshSource &source);
    // void ApplyMeshSource(MeshResourceHandle h, MeshData source, std::unique_ptr<uint8_t[]> sourceMemory);

    // bool IsHandleValid(MeshResourceHandle &h) const;

    // const Mesh* GetMesh(MeshResourceHandle h) {
    //     if (!IsHandleValid(h)) {
    //         return nullptr;
    //     }
    //     return &mesh[h.index];
    // }
    // const MeshData* GetMeshData(MeshResourceHandle h) const {
    //     if (!IsHandleValid(h)) {
    //         return nullptr;
    //     }
    //     return &meshData[h.index];
    // }

    // void SaveMeshObj(MeshResourceHandle h, std::string outFile = "");
private:
    std::shared_ptr<iAsyncLoader> async_loader;
    std::shared_ptr<iContextResourceLoader> context_loader;

    // template<typename T>
    // using Array = std::array<T, Conf::Limit>;

    // using Bitmap = ConfRes::BitmapAllocator<Conf::Limit>;

    // Bitmap allocationBitmap;
    // Array<Device::VAOHandle> deviceHandle;

    // Array<Mesh> mesh;

    // Array<Conf::VAOBuffers> vao_buffers;
    // Array<Device::VAOHandle> vao_handle;
    // Array<MeshResourceHandle::Generation_t> generations;
    // Array<MeshData> meshData;
    // Array<std::unique_ptr<uint8_t[]>> meshDataMemory;

    // std::unordered_map<std::string, MeshResourceHandle> loadedMeshes; //temporary solution

    // std::pair<MeshResourceHandle, bool> Allocate(const std::string &uri);
};

//static_assert((sizeof(MaterialManager) % 16) == 0, "Invalid size!");

} //namespace MoonGlare::Renderer::Resources

// #include "../../Configuration.Renderer.h"
// #include "../../nfRenderer.h"

// #ifdef NEED_VAO_BUILDER
// #include "VAOBuilder.h"
// #endif

namespace MoonGlare::Renderer::Resources {
#if 0

class VaoResource {
	using ConfRes = Configuration::Resources;
	using Conf = Configuration::VAO;
public:
    VAOResource();
    ~VAOResource();

	bool Allocate(Commands::CommandQueue &queue, VAOResourceHandle &out);
	void Release(Commands::CommandQueue &queue, VAOResourceHandle &h);

	bool Allocate(Frame *frame, VAOResourceHandle &out);
	void Release(Frame *frame, VAOResourceHandle &h);

	Device::VAOHandle* GetHandleArrayBase() { return &m_GLHandle[0]; }

    bool IsHandleValid(VAOResourceHandle &h) const;

#ifdef NEED_VAO_BUILDER
    Builder::VAOBuilder GetVAOBuilder(Commands::CommandQueue &q, VAOResourceHandle &h, bool AllowAllocation = false) {
		if (AllowAllocation && !IsHandleValid(h)) {
			Allocate(q, h);
		}

		return Builder::VAOBuilder {
			&q,
			&m_GLVAOBuffsers[h.index][0],
			&m_GLHandle[h.index],
		};
	}
#endif
private:
	template<typename T>
	using Array = std::array<T, Conf::VAOLimit>;
	using Bitmap = ConfRes::BitmapAllocator<Conf::VAOLimit>;
	using VAOBuffers = Conf::VAOBuffers;

	Bitmap m_AllocationBitmap;
	Array<Device::VAOHandle> m_GLHandle;
	Array<VAOBuffers> m_GLVAOBuffsers;
    Array<TextureResourceHandle::Generation_t> generations;
	ResourceManager *m_ResourceManager = nullptr;
    void* padding[2];
};

static_assert((sizeof(VAOResource) % 16) == 0, "Invalid size!");

#endif
} //namespace MoonGlare::Renderer::Resources
