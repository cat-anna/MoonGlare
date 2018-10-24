#pragma once

#include <type_traits>

namespace MoonGlare::Resources::BlobFile {


using HeaderIndex = uint8_t;

enum class MagicValue : uint32_t {
    Blob = 'BOLB', //blob 
    Mesh = 'HSEM',
    Data = 'ATAD',  
};             

struct BlobHeader {
    MagicValue magic = MagicValue::Blob;
    MagicValue contentMagic = MagicValue::Blob; // = MagicValue::[[may differ]]
    HeaderIndex headerCount = 0; // including BlobHeader

    uint8_t __padding8[3] = { };
    uint32_t __padding32[1] = {};
};
static_assert(sizeof(BlobHeader) == 16);
static_assert(std::is_standard_layout_v<BlobHeader>);
       
//Describes single mesh
struct MeshHeader {
    MagicValue magic = MagicValue::Mesh;
    uint8_t meshVersion = 0;          //for future capability
    uint8_t meshDataHeaderIndex = 0;  //index of header which describes where bytes are in file
    uint8_t blobDataHeaderIndex = 0;  //index of header which describes where bytes are in file

    uint8_t __padding8 = {};
    uint32_t __padding32[2] = {};
};
static_assert(sizeof(MeshHeader) == 16);
static_assert(std::is_standard_layout_v<MeshHeader>);

struct DataHeader {
    MagicValue magic = MagicValue::Data;
    uint32_t fileOffset = 0;
    uint32_t filedataSize = 0;
    uint32_t dataSize = 0;
};
static_assert(sizeof(DataHeader) == 16);
static_assert(std::is_standard_layout_v<DataHeader>);

}