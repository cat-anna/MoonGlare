#pragma once

#include "resources/skeletal_animation.hpp"
#include <memory>
#include <pugixml.hpp>
#include <string>

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

} // namespace MoonGlare::Resources::Importer
