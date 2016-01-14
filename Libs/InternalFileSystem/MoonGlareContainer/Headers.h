#pragma once
#ifndef idEA0ADC8C_0290_45E0_A08F3CF53EFCAAAE
#define idEA0ADC8C_0290_45E0_A08F3CF53EFCAAAE
#pragma pack(push, mgc_headers, 1)

#pragma warning ( disable: 4200 )

namespace MoonGlare {
namespace FileSystem {
namespace MoonGlareContainer {
namespace Headers {

const char ContainerEXT[] = ".mgc";

using u64 = unsigned __int64;
using u32 = unsigned __int32;
using u16 = unsigned __int16;
using u8 = unsigned __int8;


enum class Signature : u32 {
	MGCF = 'M' | 'G' << 8 | 'C' << 16 | 'F' << 24,
	RDC = 'R' | 'D' << 8 | 'C' << 16 | '!' << 24,

	FileHeader = RDC,
	FileFooter = RDC,
};

using Version = u16;

struct FileHeader {
	Signature FileSignature = Signature::FileHeader;
	Signature UserSignature = Signature::MGCF;
	Version Version = 0;
	u16 Flags = 0;

	u32 unused_32_0 = 0;

	void Reset() {
		memset(this, 0, sizeof(*this));
	}
};

struct FileHeaderFlags {
	enum {
		//nothing yet
	};
};

struct MD5CheckSum {
	union {
		u8 bytes[16];
		u32	dwords[4];
		u64 qwords[2];
	};

	bool isValid() const { return !(qwords[0] && qwords[1]); }
	bool operator ==(const MD5CheckSum& chk) const {
		return qwords[0] == chk.qwords[0] && qwords[1] == chk.qwords[1];
	}

	MD5CheckSum() {
		qwords[0] = qwords[1] = 0;
	}
};


enum class SectionType : u8 {
	EmptyEntry,/** Ignore all section of that type */

//version 0
	FileTable,
	RawData,
	StringTable,
};

enum class CompressionMode : u8 {
	None,
};

struct Compression {
	CompressionMode Mode = CompressionMode::None;
	u8 unused_8_0 = 0;
	u8 unused_8_1 = 0;
	u8 unused_8_2 = 0;
};

enum class EncryptionMode : u8 {
	None,
};

struct Encryption {
	EncryptionMode Mode = EncryptionMode::None;
	u8 unused_8_0 = 0;
	u8 unused_8_1 = 0;
	u8 unused_8_2 = 0;
};

struct DataBlock32 {
	using Size = u32;

	Encryption Encryption;
	Compression Compression;
	Size RealSize = 0;
	Size ContainerSize = 0;
	Size FilePointer = 0;
	u32 padding_32_0 = 0;
//	MD5CheckSum CheckSum;

	void Reset() {
		memset(this, 0, sizeof(*this));
	}
};

} // Headers namespace
} // MoonGlareContainer namespace
} // FileSystem namespace
} // MoonGlare namespace

#include "Headers_v0.h"

namespace MoonGlare {
namespace FileSystem {
namespace MoonGlareContainer {
namespace Headers {

namespace CurrentVersion = Version_0;

} // Headers namespace

namespace CurrentVersion = Headers::CurrentVersion;

} // MoonGlareContainer namespace
} // FileSystem namespace
} // MoonGlare namespace

#pragma pack(pop, mgc_headers)
#endif // header
