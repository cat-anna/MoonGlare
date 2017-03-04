#pragma once 

namespace MoonGlare::x2c::Settings {
struct AssetConfiguration_t;
}

namespace MoonGlare::Asset {

namespace Shader {
	class Loader;
}
namespace Texture {
	class Loader;
}

class FileSystem;
class AssetManager;

using UniqueAssetManager = std::unique_ptr<AssetManager>;

using FileHash = uint32_t;

} //namespace MoonGlare::Asset 

