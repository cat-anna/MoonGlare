/*
  * Generated by cppsrc.sh
  * On 2016-01-01 18:40:19,68
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef Builder_H
#define Builder_H

namespace StarVFS {
namespace RDC {

class Builder {
public:
 	Builder();
 	virtual ~Builder();

	bool BeginFile(const String &FileName);

	bool CloseFile();

	virtual VersionValue GetVersion() const = 0;

protected:
	virtual bool WriteFileHeader();
	virtual bool WriteSections() = 0;
	virtual bool WriteFileFooter() = 0; //and section table
	virtual void Reset() = 0;

	BlockFileDevice* GetDevice() { return m_FileDevice.get(); }
private:
	std::unique_ptr<BlockFileDevice> m_FileDevice;

	union {
		struct {
			uint8_t Opened : 1;
		};
		uint32_t intval;
	} m_Flags;
};

} //namespace RDC 
} //namespace StarVFS 

#endif