#pragma once

#include <iostream>

struct aiScene;

namespace MoonGlare::Dump {

void AssimpDump(const std::string &file, std::ostream &output);
void AssimpDump(const aiScene *scene, std::ostream &output);

}