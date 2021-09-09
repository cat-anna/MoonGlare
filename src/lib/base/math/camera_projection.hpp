#pragma once

#include "constants.hpp"
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Geometry>

namespace MoonGlare::math {

template <typename Derived>
Eigen::Matrix<typename Derived::Scalar, 4, 4> LookAt(Derived const &eye, Derived const &center,
                                                     Derived const &up) {
    typedef Eigen::Matrix<typename Derived::Scalar, 4, 4> Matrix4;
    typedef Eigen::Matrix<typename Derived::Scalar, 3, 1> Vector3;
    Vector3 f = (center - eye).normalized();
    Vector3 u = up.normalized();
    Vector3 s = f.cross(u).normalized();
    u = s.cross(f);
    Matrix4 mat = Matrix4::Zero();
    mat(0, 0) = s.x();
    mat(0, 1) = s.y();
    mat(0, 2) = s.z();
    mat(0, 3) = -s.dot(eye);
    mat(1, 0) = u.x();
    mat(1, 1) = u.y();
    mat(1, 2) = u.z();
    mat(1, 3) = -u.dot(eye);
    mat(2, 0) = -f.x();
    mat(2, 1) = -f.y();
    mat(2, 2) = -f.z();
    mat(2, 3) = f.dot(eye);
    mat.row(3) << 0, 0, 0, 1;
    return mat;
}

template <typename Scalar>
Eigen::Matrix<Scalar, 4, 4> Ortho(Scalar const &left, Scalar const &right, Scalar const &bottom,
                                  Scalar const &top, Scalar const &zNear, Scalar const &zFar) {
    Eigen::Matrix<Scalar, 4, 4> mat = Eigen::Matrix<Scalar, 4, 4>::Identity();
    mat(0, 0) = Scalar(2) / (right - left);
    mat(1, 1) = Scalar(2) / (top - bottom);
    mat(2, 2) = -Scalar(2) / (zFar - zNear);
    mat(3, 0) = -(right + left) / (right - left);
    mat(3, 1) = -(top + bottom) / (top - bottom);
    mat(3, 2) = -(zFar + zNear) / (zFar - zNear);
    return mat;
}

template <typename Scalar>
Eigen::Matrix<Scalar, 4, 4> Ortho(Scalar const &left, Scalar const &right, Scalar const &bottom,
                                  Scalar const &top) {
    Eigen::Matrix<Scalar, 4, 4> mat = Eigen::Matrix<Scalar, 4, 4>::Identity();
    mat(0, 0) = Scalar(2) / (right - left);
    mat(1, 1) = Scalar(2) / (top - bottom);
    mat(2, 2) = -Scalar(1);
    mat(3, 0) = -(right + left) / (right - left);
    mat(3, 1) = -(top + bottom) / (top - bottom);
    return mat;
}

template <typename Scalar>
Eigen::Matrix<Scalar, 4, 4> Perspective(Scalar fovy, Scalar aspect, Scalar zNear, Scalar zFar) {
    Eigen::Transform<Scalar, 3, Eigen::Projective> tr;
    tr.matrix().setZero();
    assert(aspect > 0);
    assert(zFar > zNear);
    assert(zNear > 0);
    Scalar radf = pi<Scalar> * fovy / 180.0f;
    Scalar tan_half_fovy = std::tan(radf / 2.0f);
    tr(0, 0) = 1.0f / (aspect * tan_half_fovy);
    tr(1, 1) = 1.0f / (tan_half_fovy);
    tr(2, 2) = -(zFar + zNear) / (zFar - zNear);
    tr(3, 2) = -1.0f;
    tr(2, 3) = -(2.0f * zFar * zNear) / (zFar - zNear);
    return tr.matrix();
}

} // namespace MoonGlare::math
