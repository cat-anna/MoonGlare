/*
  * Generated by cppsrc.sh
  * On 2015-12-17 21:51:26,41
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef RDCContainer_H
#define RDCContainer_H

#include "../RDC/nRDC.h"

namespace StarVFS {
namespace Containers {

class RDCContainer final : public iContainer {
public:
 	RDCContainer(FileTableInterface *fti, const String& Location = "");
 	virtual ~RDCContainer();

	virtual bool Initialize();

	virtual FileID GetFileCount() const override;
	virtual bool ReloadContainer() override;
	virtual bool RegisterContent() const override;

	virtual bool GetFileData(FileID ContainerFID, ByteTable &out) const override;
	virtual FileID FindFile(const String& ContainerFileName) const override;

	virtual String GetContainerURI() const override;
	virtual RWMode GetRWMode() const override { return RWMode::RW; };

	static bool CanOpen(const String&);
	static CreateContainerResult CreateFor(StarVFS *svfs, const String& MountPoint, const String& Location);

	RDC::Version_1::Reader_v1* GetRDCReader() { return m_Reader.get(); }
private: 
	union {
		struct {
			uint8_t HasMountEntry : 1;
			uint8_t HasOffsetTable : 1;
		};
		uint32_t uintval;
	} m_Flags;
	String m_FileName;
	std::unique_ptr<RDC::Version_1::Reader_v1> m_Reader;
	RDC::Version_1::MountEntryInfo m_MountEntryInfo;

	RDC::Version_1::DataBlock m_RawSectionBaseBlock;
	RDC::Version_1::OffsetDataBlockTable m_OffsetTable;
};

} //namespace Containers 
} //namespace StarVFS 

#endif
