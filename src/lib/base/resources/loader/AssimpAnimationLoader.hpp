#pragma once

#include "resources/async_loader.hpp"
#include "resources/skeletal_animation_manager.hpp"
#include <memory>
#include <pugixml.hpp>
#include <string>

struct aiScene;
namespace Assimp {
class Importer;
}

namespace MoonGlare::Resources::Loader {

class AssimpAnimationLoader : public MultiAsyncFileSystemRequest {
public:
    AssimpAnimationLoader(std::string subpath, SkeletalAnimationHandle handle, SkeletalAnimationManager &Owner);
    ~AssimpAnimationLoader();

    void OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata) override;

private:
    SkeletalAnimationManager &owner;
    SkeletalAnimationHandle handle;

    int GetAnimIndex() const;

    std::string subpath;
    std::string ModelURI;
    std::string baseURI;
    std::unique_ptr<Assimp::Importer> importer;
    const aiScene *scene;
    pugi::xml_document animSetXmlDoc;
    void LoadAnimation();
};

} // namespace MoonGlare::Resources::Loader
