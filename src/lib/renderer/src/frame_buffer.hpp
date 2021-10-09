#pragma once

#include "base_render_target.hpp"

namespace MoonGlare::Renderer {

class alignas(16) FrameBuffer : public BaseRenderTarget {
public:
    FrameBuffer(gsl::not_null<iEngineTime *> engine_time,
                gsl::not_null<CommandQueue *> command_queue,
                gsl::not_null<iResourceManager *> _resource_manager, math::ivec2 display_size);
    ~FrameBuffer() override = default;
};

} // namespace MoonGlare::Renderer
