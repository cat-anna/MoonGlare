#pragma once

namespace MoonGlare::Renderer::Configuration {

struct FrameBuffer {
	static constexpr uint32_t MemorySize = 1 * 1024 * 1024;
	static constexpr uint32_t Count = 3;
};

struct CommandBucket {
	static constexpr uint32_t MaxCommandsPerBucket = 16;
	using BucketCommandCount = uint8_t;
	using BucketCommandIndex = uint16_t;
	static constexpr uint32_t MaxBuckets = 1024;
	static constexpr uint32_t MaxCommands = 4096;

	static constexpr uint32_t ArgumentMemoryBuffer = 512 * 1024;
struct CommandQueue {
	static constexpr uint32_t ArgumentMemoryBuffer = 1 * 128 * 1024; //1mb
	static constexpr uint32_t CommandLimit = 4096;
	static constexpr uint32_t BytesPerCommand = ArgumentMemoryBuffer / CommandLimit;
};

};

} //namespace MoonGlare::Renderer::Configuration
