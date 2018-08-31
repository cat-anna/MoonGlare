#pragma once

#include <type_traits>

namespace MoonGlare::Resources::MeshFile {

enum class MagicValue : uint32_t {
    Header = 'HSEM',  //MESH   for Header
    Data = 'ATAD',    //DATA   for DataHeader
};             
                                             
enum DataDestination : uint8_t {
    invalid, 
    Coordinates, TexMapping, Normals, Tangents,
    TriangleIndex, QuadIndex, LineIndex, 
    BoundingBox, BoundingSphere,
    MaxValue,
};
            
enum DataType : uint8_t {
    invalid, 
    u8, s8, u16, s16, u32, s32, u64, s64, 
    Float, Double,
    MaxValue,
};

struct Header {
    MagicValue magic;
    uint16_t headerSectionSize;
    uint16_t dataHeaderCnt;

    uint32_t renderableDataSize;   //data which can be directly send to opengl buffer, 
    uint32_t renderableDataOffset;
};
static_assert(sizeof(Header) == 16);
static_assert(std::is_pod_v<Header>);

struct DataHeader {
    MagicValue magic;

    DataType dataType;              //data type of single value
    DataDestination dataDestination;
    uint8_t elementSize : 4;	   //number of components per generic element ie 1,2,3,4
    uint8_t channel : 4;
    uint8_t dataStride;		       //Specifies the byte offset between consecutive generic element	

    uint32_t byteSize;
    uint32_t byteOffset;           //from the beginning of file
};
static_assert(sizeof(DataHeader) == 16);
static_assert(std::is_pod_v<DataHeader>);

}