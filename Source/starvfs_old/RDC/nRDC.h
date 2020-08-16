#pragma once
#ifndef nRDC_H
#define nRDC_H

#include "../nfStarVFS.h"

namespace StarVFS {
namespace RDC {

class BlockFileDevice;
using UniqueBlockFileDevice = std::unique_ptr<BlockFileDevice>;

struct Settings {
	enum {
		BlockAlignmentValue = 8,
	};
};

} //namespace RDC 
} //namespace StarVFS 

//#ifdef _MSC_VER
#pragma pack(push, RDC_headers, 1)
//#else
//#error not supported compiler!
//#endif

#include "Headers.h"
#include "Version_1/Headers_v1.h"

//#ifdef _MSC_VER
#pragma pack(pop, RDC_headers)
//#endif

#include "BlockFileDevice.h"
#include "Builder.h"
#include "Reader.h"

#include "Version_1/BlockProcessor_v1.h"
#include "Version_1/Sections_v1.h"
#include "Version_1/Reader_v1.h"
#include "Version_1/Builder_v1.h"

namespace StarVFS {
namespace RDC {

} //namespace RDC 
} //namespace StarVFS 

#endif
