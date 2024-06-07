#pragma once

#include "math/camera_projection.hpp"
#include "math/vector.hpp"
#include <glm/glm.hpp>

namespace MoonGlare::Renderer {

struct alignas(16) VirtualCamera {
    math::fmat4 projection_matrix;

    // void SetPerspective(float aspect, float fov = 45.0f, float near = 0.1f, float far = 1.0e4f) {
    //     // projection_matrix =
    //     // // math::MathCast<math::fmat4>(glm::perspective(glm::radians(fov), aspect, near, far));
    // }
    // void SetOrthogonal(float width, float height) {
    //     projection_matrix = math::Ortho(0.0f, width, height, 0.0f);
    // }
    // void SetOrthogonalUniform(float width, float height) {
    //     // projection_matrix = math::MathCast<math::fmat4>(glm::ortho(-width, width, height, -height));
    // }
    // void SetOrthogonalRect(float left, float top, float right, float bottom) {
    //     // projection_matrix = math::MathCast<math::fmat4>(glm::ortho(left, right, bottom, top));
    // }
    // void SetOrthogonalRect(float left, float top, float right, float bottom, float near,
    //                        float far) {
    //     // projection_matrix =
    //     // math::MathCast<math::fmat4>(glm::ortho(left, right, bottom, top, near, far));
    // }

    // void SetDefaultPerspective(const math::fvec2 &screen_size) {
    //     SetPerspective(screen_size[0] / screen_size[1]);
    // }

    // void SetDefaultOrthogonal(const math::fvec2 &screen_size) {
    //     SetOrthogonal(screen_size[0], screen_size[1]);
    // }

    void SetUniformOrthogonal(float aspect) {
        projection_matrix = math::Ortho(-aspect, aspect, 1.0f, -1.0f);
    }

    void SetUniformOrthogonal(const math::fvec2 &screen_size) {
        SetUniformOrthogonal(screen_size[0] / screen_size[1]);
    }
};

} //namespace MoonGlare::Renderer
