#pragma once

#include <Foundation/Resources/SkeletalAnimation.h>

#include <gsl/span>

namespace MoonGlare::Resources::Blob {

struct AnimationLoad {
    std::unique_ptr<uint8_t[]> memory;
    SkeletalAnimation animation = {};
};

void WriteAnimationBlob(std::ostream& output, const SkeletalAnimation &animationData);
bool ReadAnimationBlob(gsl::span<uint8_t> memory, AnimationLoad &output);

void DumpAnimationBlob(const SkeletalAnimation &animationDat, const std::string name = "");

}
