#pragma once

#include <type_traits>

namespace MoonGlare::Resources::Blob {

using HeaderIndex = uint8_t;

enum class MagicValue : uint32_t {
    Blob = 'BOLB', 
    Mesh = 'HSEM',
    Anim = 'MINA',
    Data = 'ATAD',  
    Null = 'LLUN', // empty entry
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
       
struct MeshHeader {
    MagicValue magic = MagicValue::Mesh;
    HeaderIndex meshDataHeaderIndex = 0;  //index of header which describes where bytes are in file
    HeaderIndex blobDataHeaderIndex = 0;  //index of header which describes where bytes are in file
    uint8_t meshVersion = 0;              //for future capability
    uint8_t meshType = 0;                 //for future capability

    uint32_t __padding32[2] = {};
};
static_assert(sizeof(MeshHeader) == 16);
static_assert(std::is_standard_layout_v<MeshHeader>);

struct AnimHeader {
    MagicValue magic = MagicValue::Anim;
    HeaderIndex animDataHeaderIndex = 0;  //index of header which describes where bytes are in file
    HeaderIndex blobDataHeaderIndex = 0;  //index of header which describes where bytes are in file
    uint8_t animVersion = 0;              //for future capability
    uint8_t animType = 0;                 //for future capability

    uint32_t __padding32[2] = {};
};
static_assert(sizeof(AnimHeader) == 16);
static_assert(std::is_standard_layout_v<AnimHeader>);

struct DataHeader {
    MagicValue magic = MagicValue::Data;
    uint32_t fileOffset = 0;
    uint32_t fileDataSize = 0;
    uint32_t dataSize = 0;
};
static_assert(sizeof(DataHeader) == 16);
static_assert(std::is_standard_layout_v<DataHeader>);

}
