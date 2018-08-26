#pragma once

namespace MoonGlare::Renderer::Resources::Loader {

class CustomMeshLoader : public iAsyncTask {              
public:
    CustomMeshLoader(MeshResourceHandle handle, MeshManager &Owner) :
        owner(Owner), handle(handle) { }

    void Do(ResourceLoadStorage &storage) override;

    Mesh meshArray;

    MaterialResourceHandle materialArray;

private:
    MeshManager &owner;
    MeshResourceHandle handle;
};

} //namespace MoonGlare::Renderer::Resources::Loader 
