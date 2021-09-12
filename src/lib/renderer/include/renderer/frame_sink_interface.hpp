#pragma once

#include "math/vector.hpp"
#include "types.hpp"

namespace MoonGlare::Renderer {

class FrameBuffer;

class iFrameSink {
public:
    virtual ~iFrameSink() = default;

    //virtual math::fvec2 DisplaySize() const = 0;//?
    virtual void SetFrameBuffer(FrameBuffer *frame_buffer) = 0;

    struct ElementReserve {
        size_t index_count;
        size_t vertex_count;
        size_t texture0_count;
    };
    struct ElementBuffer {
        const ElementReserve reserve;
        RenderElementIndexType *const index_buffer = nullptr;
        math::fvec3 *const vertex_buffer = nullptr;
        math::fvec2 *const texture0_buffer = nullptr;
    };
    struct ElementRenderRequest {
        math::fmat4 position_matrix;
        GLenum element_mode;
        size_t index_count;
        //TODO
        // int material_handle
        ShaderHandle shader_handle;
        TextureHandle texture_handle = kInvalidTextureHandle;
        //layer?
    };

    virtual ElementBuffer ReserveElements(const ElementReserve &config) = 0;
    virtual void SubmitElements(const ElementBuffer &buffer,
                                const ElementRenderRequest &request) = 0;
};

} // namespace MoonGlare::Renderer
