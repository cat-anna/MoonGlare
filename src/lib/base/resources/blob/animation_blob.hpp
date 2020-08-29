#pragma once

#include "resources/skeletal_animation.hpp"
#include <gsl/span>
#include <memory>
#include <string>

namespace MoonGlare::Resources::Blob {

struct AnimationLoad {
    std::unique_ptr<uint8_t[]> memory;
    SkeletalAnimation animation = {};
};

void WriteAnimationBlob(std::ostream &output, const SkeletalAnimation &animationData);
bool ReadAnimationBlob(gsl::span<uint8_t> memory, AnimationLoad &output);

void DumpAnimationBlob(const SkeletalAnimation &animationDat, const std::string name = "");

} // namespace MoonGlare::Resources::Blob
