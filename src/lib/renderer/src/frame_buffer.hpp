#pragma once

#include "base_render_target.hpp"

namespace MoonGlare::Renderer {

class alignas(16) FrameBuffer : public BaseRenderTarget {
public:
    FrameBuffer(CommandQueue *command_queue, math::ivec2 display_size);
    ~FrameBuffer() override = default;
};

} // namespace MoonGlare::Renderer
