#include "BlobIo.h"

namespace MoonGlare::Resources::Blob {

bool BlobReader::ReadDataBlob(gsl::span<const uint8_t> BlobMemory, const DataHeader &header, gsl::span<uint8_t> outputMemory) {
    assert(!BlobMemory.empty());
    if (outputMemory.size_bytes() < header.dataSize)
        return false;

    const auto front = BlobMemory.data() + header.fileOffset;
    std::copy(front, front + header.fileDataSize, outputMemory.data());
    return true;
}

bool BlobReader::ReadDataBlob(gsl::span<const uint8_t> BlobMemory, const DataHeader &header, std::unique_ptr<uint8_t[]> &outputMemory) {
    outputMemory.reset(new uint8_t[header.dataSize]);
    return ReadDataBlob(BlobMemory, header, { outputMemory.get(), (ptrdiff_t)header.dataSize });
}

}
