
#include "AnimationBlob.h"

namespace MoonGlare::Resources::Blob {

void WriteAnimationBlob(std::ostream& output, const SkeletalAnimation *animationData) {
    auto data = *animationData;
    //data.UpdatePointers(0x1000'0000);

    size_t fileOffset = 0;

    BlobHeader blobH = { MagicValue::Blob, MagicValue::Anim, 4 };
    AnimHeader animH = { MagicValue::Anim, 2, 3 };
    DataHeader animDataH = { MagicValue::Data, 0, 0, 0 };
    DataHeader blobDataH = { MagicValue::Data, 0, 0, 0 };

    fileOffset = sizeof(blobH) + sizeof(animH) + sizeof(animDataH) + sizeof(blobDataH);

    animDataH.dataSize = sizeof(data);
    animDataH.fileDataSize = sizeof(data);
    animDataH.fileOffset = fileOffset;
    fileOffset += animDataH.fileDataSize;

    blobDataH.dataSize = animationData->memoryBlockSize;
    blobDataH.fileDataSize = animationData->memoryBlockSize;
    blobDataH.fileOffset = fileOffset;
    fileOffset += blobDataH.fileDataSize;

    output.write((char*)&blobH, sizeof(blobH));
    output.write((char*)&animH, sizeof(animH));
    output.write((char*)&animDataH, sizeof(animDataH));
    output.write((char*)&blobDataH, sizeof(blobDataH));
    output.write((char*)&data, sizeof(data));
    output.write((char*)animationData->memoryBlockFront, blobDataH.fileDataSize);
}

}
