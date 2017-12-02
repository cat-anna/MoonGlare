#include <cstddef>
#include <string>
#include <memory>
#include <vector>
#include <list>
#include <mutex>
#include <iostream>

#include "arguments.h"
#include "rdcc.h"

int main(int argc, char **argv) {
    auto config = MoonGlare::RDCC::ArgumentParser::Run(argc, argv);
    MoonGlare::RDCC::RDCC rdcc(config);
	return rdcc.Run();
}
