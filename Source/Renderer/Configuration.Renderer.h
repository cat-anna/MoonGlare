#pragma once

namespace MoonGlare::Renderer::Configuration {

struct FrameBuffer {
	static constexpr uint32_t MemorySize = 1 * 128 * 1024;
	static constexpr uint32_t Count = 3;
};

struct CommandBucket {
	static constexpr uint32_t MaxCommandsPerBucket = 16;
	using BucketCommandCount = uint8_t;
	using BucketCommandIndex = uint16_t;
	static constexpr uint32_t MaxBuckets = 1024;
	static constexpr uint32_t MaxCommands = 4096;

	static constexpr uint32_t ArgumentMemoryBuffer = 1 * 128 * 1024;
};

struct CommandQueue {
	static constexpr uint32_t ArgumentMemoryBuffer = 1 * 128 * 1024; //1mb
	static constexpr uint32_t CommandLimit = 4096;
	static constexpr uint32_t BytesPerCommand = ArgumentMemoryBuffer / CommandLimit;
};

struct IndexBuffer {
	static constexpr uint32_t TextureBuffer = 256;
};
struct Resources {
	template<size_t SIZE>
	using BitmapAllocator = ::Space::Memory::LinearAtomicBitmapAllocator<SIZE, uint32_t, uint32_t>;

	static constexpr uint32_t TextureLimit = 1024;
	static constexpr uint32_t TextureInitial = 64;

	static constexpr uint32_t VAOLimit = 1024;
	static constexpr uint32_t VAOInitial = 64;
};

struct TextureRenderTask {
	static constexpr uint32_t Limit = 64;
};

} //namespace MoonGlare::Renderer::Configuration
