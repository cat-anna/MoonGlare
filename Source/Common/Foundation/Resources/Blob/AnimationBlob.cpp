
#include "BlobHeaders.h"
#include "AnimationBlob.h"
#include "BlobIo.h"

namespace MoonGlare::Resources::Blob {

void WriteAnimationBlob(std::ostream& output, const SkeletalAnimation &animationData) {
    auto data = animationData;
    data.UpdatePointers(0x1000'0000);

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

    blobDataH.dataSize = animationData.memoryBlockSize;
    blobDataH.fileDataSize = animationData.memoryBlockSize;
    blobDataH.fileOffset = fileOffset;
    fileOffset += blobDataH.fileDataSize;

    output.write((char*)&blobH, sizeof(blobH));
    output.write((char*)&animH, sizeof(animH));
    output.write((char*)&animDataH, sizeof(animDataH));
    output.write((char*)&blobDataH, sizeof(blobDataH));
    output.write((char*)&data, sizeof(data));
    output.write((char*)animationData.memoryBlockFront, blobDataH.fileDataSize);
}

bool ReadAnimationBlob(gsl::span<uint8_t> memory, AnimationLoad &output) {
    assert(!memory.empty());

    if (memory.size_bytes() < sizeof(BlobHeader)) {
        AddLogf(Error, "Cannot interpret animation blob: empty data");
        return false;
    }

    const GenericHeader *headers = reinterpret_cast<const GenericHeader*>(memory.data());

    const BlobHeader *bh = reinterpret_cast<const BlobHeader*>(headers + 0);
    if (bh->magic != MagicValue::Blob || bh->contentMagic != MagicValue::Anim || bh->headerCount != 4) {
        AddLogf(Error, "Cannot interpret animation blob: invalid headers");
        return false;
    }

    if (memory.size_bytes() <= bh->headerCount * HeaderSize) {
        AddLogf(Error, "Cannot interpret animation blob: invalid header size");
        return false;
    }

    const AnimHeader *ah = reinterpret_cast<const AnimHeader*>(headers + 1);
    if (ah->magic != MagicValue::Anim || std::max(ah->blobDataHeaderIndex, ah->animDataHeaderIndex) >= bh->headerCount) {
        AddLogf(Error, "Cannot interpret animation blob: invalid secondary header");
        return false;
    }

    const DataHeader *dhBlob = reinterpret_cast<const DataHeader*>(headers + ah->blobDataHeaderIndex);
    const DataHeader *dhAnim = reinterpret_cast<const DataHeader*>(headers + ah->animDataHeaderIndex);

    if (dhBlob->magic != MagicValue::Data || dhAnim->magic != MagicValue::Data) {
        AddLogf(Error, "Cannot interpret animation blob: invalid data headers");
        return false;
    }

    size_t fileSize = std::max(dhBlob->fileOffset + dhBlob->fileDataSize, dhAnim->fileOffset + dhAnim->fileDataSize);
    if (memory.size_bytes() < fileSize) {
        AddLogf(Error, "Cannot interpret animation blob: invalid data size");
        return false;
    }

    //TODO: check header version
    BlobReader br(bh);

    if (dhAnim->fileDataSize != sizeof(output.animation)) {
        AddLogf(Error, "Cannot interpret animation blob: invalid anim size");
        return false;
    }

    bool r1 = br.ReadDataBlob(memory, *dhAnim, { (uint8_t*)&output.animation , sizeof(output.animation) });
    bool r2 = br.ReadDataBlob(memory, *dhBlob, output.memory);

    if (!r1 || !r2) {
        AddLogf(Error, "Cannot interpret mesh blob: failure during data reading");
        return false;
    }

    output.animation.UpdatePointers((intptr_t)output.memory.get());
    if (!output.animation.CheckPointers()) {
        AddLogf(Error, "Cannot interpret animation blob: pointer validation failed");
        return false;
    }

    return true;
}

void DumpAnimationBlob(const SkeletalAnimation &animationDat, const std::string name) {
    static int animIndex = 0;

    std::string outFile = "logs/animation.";
    if (name.empty())
        outFile += std::to_string(animIndex++);
    else
        outFile += name;
    outFile += ".anim";

    std::ofstream of(outFile, std::ios::out | std::ios::binary);
    Blob::WriteAnimationBlob(of, animationDat);
}                                                                                   

}
