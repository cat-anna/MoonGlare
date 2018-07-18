#define _WIN32_WINNT 0x0600

#include <Windows.h> // for MAX_PATH
#include <shlobj.h> // for getFolderPath function

#include "path.h"

#include "boost/filesystem.hpp"

namespace MoonGlare::OS {

std::string GetSettingsDirectory(const std::string &Product)
{
#ifdef WINDOWS
    char szPath[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, szPath) != S_OK)
    {
        throw std::runtime_error("SHGetFolderPath failed!");
    }      

    std::string ret = szPath;
    if(ret.back() != '\\')
        ret += "\\";
    ret += Product;
    ret += "\\";

    boost::filesystem::create_directories(ret);

    return ret;
#else
#error Only windows is supported
#endif
}

std::string GetSettingsFilePath(const std::string &Name, const std::string &Product) {
    return GetSettingsDirectory() + Name;
}

}
