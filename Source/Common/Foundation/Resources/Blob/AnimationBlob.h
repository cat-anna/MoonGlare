#pragma once

#include <Foundation/Resources/SkeletalAnimation.h>

namespace MoonGlare::Resources::Blob {

void WriteAnimationBlob(std::ostream& output, const SkeletalAnimation &animationData);

void DumpAnimationBlob(const SkeletalAnimation &animationDat, const std::string name = "");

}