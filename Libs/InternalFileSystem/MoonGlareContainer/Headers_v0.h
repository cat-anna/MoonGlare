#pragma once

namespace MoonGlare {
namespace FileSystem {
namespace MoonGlareContainer {
namespace Headers {
namespace Version_0 {

using namespace ::MoonGlare::FileSystem::MoonGlareContainer::Headers;

using SectionIndex = unsigned __int8;
using SectionPointer = unsigned __int32;
using FileIndex = unsigned __int32;

using DataBlock = DataBlock32;
using Size = DataBlock::Size;
using FilePointer = DataBlock::Size;

struct FileFooter {
	DataBlock SectionsBlock;
	/** Count of sections */
	SectionIndex SectionCount = 0;
	/** Unused */
	u8 unused_8_0 = 0;
	u8 unused_8_1 = 0;
	u8 unused_8_2 = 0;
//	MD5CheckSum CheckSum;
	/** Footer signature */
	Signature Signature = Signature::FileFooter;

	void Reset() {
		memset(this, 0, sizeof(*this));
	}
};

struct Version {
	enum {
		Value = 1,
	};
};

struct SectionHeader {
	/** Section blok desc */
	DataBlock SectionBlock;
	/** Section type */
	SectionType Type = SectionType::EmptyEntry;
	/** Padding */
	u8 unused_8_0 = 0;
	u8 unused_8_1 = 0;
	u8 unused_8_2 = 0;
	u32 unused_32_0 = 0;
};

//------------ file table section ------------

struct FileTableEntryFlags {
	enum {
		Folder			= 0x0001,	/** Entry describes directory. No data, and ChildrenCount contain valid value */
		SymLink			= 0x0100,	/** File data is shared between multiple files */
		HasSymLink		= 0x0200,	/** file has symlinks pointing to it */
		InvalidEntry	= 0x1000,
	};
};

struct FileTableEntry {
	/** File data block */
	DataBlock FileBlock;
	/** File name offset in string table */
	SectionPointer NamePointer;
	/** parent file index */
	FileIndex ParentIndex;
	/** Symlink file index */
	FileIndex SymLinkIndex;
	/** FileFlags */
	u16 Flags;

	u16 unused_16_0 = 0;
	u64 unused_64_0 = 0;
};

struct FileTableSection {
	/** Index of data section */
	SectionIndex RawDataSection;
	/** Index of String table section */
	SectionIndex StringTableSection;
	/** Some flags. Currently unused */
	u16 Flags;
	/** Elements in table */
	u32 Count;
	/** Space for future use */
	u64 unused_64_0;
	/** The Table */
	FileTableEntry Table[0];
};

//------------ raw data section ------------

struct RawDataSection {
	/** The Table */
	u8 Table[0];
};

//------------ string table section ------------

struct StringTableSection {
	/** The Table */
	char Table[0];
};

} // Version_0 namepace
} // Headers namespace
} // MoonGlareContainer namespace
} // FileSystem namespace
} // MoonGlare namespace
