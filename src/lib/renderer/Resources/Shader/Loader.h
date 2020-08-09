#pragma once

#include <Foundation/iFileSystem.h>

#include "ShaderCodeLoader.h"

namespace MoonGlare::Renderer::Resources::Shader {

struct ShaderFileCache;

class Loader final : public ShaderCodeLoader {
public:
 	Loader(iFileSystem *fs);
 	~Loader();

	virtual bool LoadCode(const std::string &Name, ShaderCode &Output) override;
protected:
    iFileSystem* GetFileSystem() { return fileSystem; }
private: 
	iFileSystem *fileSystem;
	std::unique_ptr<ShaderFileCache> fileCache;
};

}