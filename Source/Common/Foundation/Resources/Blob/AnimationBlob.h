#pragma once

#include "BlobHeaders.h"
#include <Foundation/Resources/SkeletalAnimation.h>

namespace MoonGlare::Resources::Blob {

void WriteAnimationBlob(std::ostream& output, const SkeletalAnimation *animationData);

}
