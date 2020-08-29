#pragma once

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Geometry>

namespace MoonGlare::math {

using fvec2 = Eigen::Vector2f;
using fvec3 = Eigen::Vector3f;
using fvec4 = Eigen::Vector4f;

using ivec2 = Eigen::Vector2i;
using ivec3 = Eigen::Vector3i;
using ivec4 = Eigen::Vector4i;

using Transform = Eigen::Affine3f;
using Quaternion = Eigen::Quaternionf;

using fmat3 = Eigen::Matrix3f;
using fmat4 = Eigen::Matrix4f;

using usvec2 = Eigen::Matrix<uint16_t, 2, 1>;
using usvec3 = Eigen::Matrix<uint16_t, 3, 1>;
using usvec4 = Eigen::Matrix<uint16_t, 4, 1>;

static_assert(sizeof(fvec3) == 3 * sizeof(float));
static_assert(sizeof(Quaternion) == 4 * sizeof(float));

} // namespace MoonGlare::math