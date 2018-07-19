#include "File.h"

#include "boost/filesystem.hpp"

namespace MoonGlare::OS {

void WriteStringToFile(const std::string &FileName, const std::string &data) {
    std::ofstream out(FileName, std::ios::out | std::ios::binary);
    out.write(data.c_str(), data.length());
    out.close();
}

bool GetFileContent(const std::string &FileName, std::string &data) {
    if (!boost::filesystem::is_regular_file(FileName))
        return false;

    auto size = boost::filesystem::file_size(FileName);
    std::ifstream in(FileName, std::ios::in | std::ios::binary);
    data.resize(size);
    in.read(const_cast<char*>(data.c_str()), size);
    in.close();
    return true;
}

}
