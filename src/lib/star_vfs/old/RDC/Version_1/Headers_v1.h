#pragma once

namespace StarVFS {
namespace RDC {
namespace Version_1 {

namespace Headers {}
using namespace Headers;

namespace Headers {

using SectionIndex = u8;
using SectionPointer = u32;
using FilePointer = u32;
using FileIndex = u16;
using StringPointer = u32;
using Size = u32;

struct CompressionInfo {
	RDC_HEADER_STRUCTURE(CompressionInfo);

	CompressionMode Mode = CompressionMode::None;
	u8 Level = 0;
	u8 unused_8_1 = 'c';//UNUSED8;
	u8 unused_8_2 = 'm';//UNUSED8;
	u32 RawSize = 0;
};
static_assert(sizeof(CompressionInfo) == 8, "DataBlock has invalid size!");

struct EncryptionInfo {
	RDC_HEADER_STRUCTURE(EncryptionInfo);

	EncryptionMode Mode = EncryptionMode::None;
	u8 unused_8_0 = 'e';//UNUSED8;
	u8 unused_8_1 = 'n';//UNUSED8;
	u8 unused_8_2 = 'c';//UNUSED8;
};
static_assert(sizeof(EncryptionInfo) == 4, "DataBlock has invalid size!");

struct CheckSumInfo {
	RDC_HEADER_STRUCTURE(CheckSumInfo);

	CheckSumMode Mode = CheckSumMode::None;
	u8 unused_8_0 = 'c';//UNUSED8;
	u8 unused_8_1 = 'h';//UNUSED8;
	u8 unused_8_2 = 'k';//UNUSED8;
};
static_assert(sizeof(CheckSumInfo) == 4, "DataBlock has invalid size!");

struct BaseDataBlock {
	RDC_HEADER_STRUCTURE(BaseDataBlock);

	CompressionInfo Compression;
	EncryptionInfo Encryption;
	CheckSumInfo CheckSum;
	Size ContainerSize = 0;
	u32 unused_32_0 = UNUSED32;

	Size GetRawSize() const { return Compression.Mode == CompressionMode::None ? ContainerSize : Compression.RawSize; }
};

struct DataBlock : public BaseDataBlock {
	RDC_HEADER_STRUCTURE(DataBlock);

	FilePointer FileLocation = 0;
	u32 unused_32_1 = UNUSED32;
};
static_assert(sizeof(DataBlock) % 8 == 0, "DataBlock has invalid size!");

struct OffsetDataBlock : BaseDataBlock {
	RDC_HEADER_STRUCTURE(OffsetDataBlock);

	FilePointer SectionOffset = 0;
	u32 unused_32_1 = UNUSED32;
};
static_assert(sizeof(OffsetDataBlock) % 8 == 0, "OffsetDataBlock has invalid size!");

struct FileFooter {
	RDC_HEADER_STRUCTURE(FileFooter);


	DataBlock SectionTableBlock;
	SectionIndex SectionCount = 0; ///Count of sections
	u8 unused_8_0 = UNUSED8;
	u8 unused_8_1 = UNUSED8;
	u8 unused_8_2 = UNUSED8;
	Signature FooterSignature = Signature::Footer; ///Footer signature
};
static_assert(sizeof(FileFooter) % 8 == 0, "FileFooter has invalid size!");

struct SectionDescriptor {
	RDC_HEADER_STRUCTURE(SectionDescriptor);

	/** descriptor of occupied data in file */
	DataBlock SectionBlock;
	/** Section type */
	SectionType Type = SectionType::EmptyEntry;
	u8 unused_8_0 = UNUSED8;
	u8 unused_8_1 = UNUSED8;
	u8 unused_8_2 = UNUSED8;
	u32 unused_32_0 = UNUSED32;
};
static_assert((sizeof(SectionDescriptor) % 8) == 0, "SectionDescriptor has invalid size!");

struct MountEntrySection {
	RDC_HEADER_STRUCTURE(MountEntrySection);

	SectionIndex StringTable = 0;
	SectionIndex RawDataSection = 0;
	SectionIndex DataBlockTable = 0;
	SectionIndex StructureSection = 0;
	SectionIndex HashTable = 0;
	u8 unused_8_0 = UNUSED8;
	u16 unused_16_0 = UNUSED16;
	StringPointer MountEntryName = 0;
	u16 MountEntryId = 0;
	u16 unused_16_1 = UNUSED16;
};
static_assert(sizeof(MountEntrySection) % 8 == 0, "MountEntrySection has invalid size!");

struct BaseFileInfo {
	RDC_HEADER_STRUCTURE(BaseFileInfo);

	StringPointer NamePointer = 0;
	FileIndex ParentIndex = 0;
	FileIndex SymLinkIndex = 0;
	u8 Flags;
	u8 unused_8_0 = UNUSED8;
	u16 unused_16_0 = UNUSED16;
	u32 unused_32_0 = UNUSED32;

	struct FlagBits {
		enum {
			Directory = 0x01,
			SymLink = 0x02,
		};
	};
};
static_assert(sizeof(BaseFileInfo) % 8 == 0, "BaseFileInfo has invalid size!");

using HashSectionItemType = FilePathHash;

} //namespace Headers 
} //namespace Version_1
} //namespace RDC 
} //namespace StarVFS 
