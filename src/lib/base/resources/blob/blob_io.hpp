#pragma once

#include "blob_headers.hpp"
#include <gsl/gsl>
#include <gsl/span>
#include <memory>
#include <type_traits>

namespace MoonGlare::Resources::Blob {

struct BlobReader {
    BlobReader(const BlobHeader *blobHeader) : blobHeader(blobHeader) {}

    bool ReadDataBlob(gsl::span<const uint8_t> BlobMemory, const DataHeader &header, gsl::span<uint8_t> outputMemory);
    bool ReadDataBlob(gsl::span<const uint8_t> BlobMemory, const DataHeader &header,
                      std::unique_ptr<uint8_t[]> &outputMemory);

private:
    const BlobHeader *blobHeader;
};

} // namespace MoonGlare::Resources::Blob
