#pragma once
#ifndef idEA0ADC8C_0290_45E0_A08F3CF53EFCAAAE
#define idEA0ADC8C_0290_45E0_A08F3CF53EFCAAAE

#include <cstdint>

#ifdef DEBUG
#define UNUSED32 ('3' | '2' << 8 | '3' << 16 | '2' << 24)
#define UNUSED16 ('1' | '6' << 8)
#define UNUSED8 ('8')
#else
#define UNUSED32 0
#define UNUSED16 0
#define UNUSED8 0
#endif

namespace StarVFS {
namespace RDC {

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;

#define RDC_HEADER_STRUCTURE(NAME)\
	using ThisClass = NAME; \
	void Zero() { memset(this, 0, sizeof(*this)); }\
	void Reset() { *this = ThisClass(); }

struct VersionValue {
	u8 Major = 0;
	u8 Minor = 0;

	static VersionValue Make(u8 major, u8 minor) { VersionValue v; v.Major = major; v.Minor = minor; return v; }
};

enum class Signature : u32 {
	RDC = 'R' | 'D' << 8 | 'C' << 16 | '!' << 24,
	FDC = 'F' | 'D' << 8 | 'C' << 16 | '!' << 24,

	Header = RDC,
	Footer = RDC,
};

struct FileHeader {
	Signature FileSignature = Signature::Header;
	VersionValue Version;
	union {
//		struct {
//		};
		u16 intval = 0;
	} Flags;

	u32 unused_32_0 = UNUSED32;
	u32 unused_32_1 = UNUSED32;
};
static_assert(sizeof(FileHeader) % 16 == 0, "FileHeader has invalid size!");

// There always must be an EmptyEntry section at the end of section table
enum class SectionType : u8 {
	EmptyEntry,/** Ignore all section of that type */

	MountEntry,

	RawData,
	OffsetDataBlockTable,
	StringTable,
	FileStructureTable,
	HashTable,
};

enum class CompressionMode : u8 {
	None,
	ZLib,
};

enum class EncryptionMode : u8 {
	None,
};

enum class CheckSumMode : u8 {
	None,
};

#if 0

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

#endif

} //namespace RDC 
} //namespace StarVFS 

#endif // header
