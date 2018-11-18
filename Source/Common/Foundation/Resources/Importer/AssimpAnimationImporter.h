#pragma once

#include <string>
#include <memory>

#include <Foundation/Resources/SkeletalAnimation.h>

struct aiScene;
namespace Assimp {
class Importer;
}

namespace MoonGlare::Resources::Importer {

struct AnimationImport {
    std::unique_ptr<uint8_t[]> memory;
    SkeletalAnimation animation = {};
};

void ImportAssimpAnimation(const aiScene *scene, int animIndex, pugi::xml_node animSetXml, AnimationImport &output);

}
