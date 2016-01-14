/*
  * Generated by cppsrc.sh
  * On 2015-02-26  1:07:44,66
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef iFileReader_H
#define iFileReader_H

namespace MoonGlare {
namespace FileSystem {

class iFileReader : public GabiLib::GabiObject {
	GABI_DECLARE_ABSTRACT_CLASS(iFileReader, GabiLib::GabiObject)
public:
 	iFileReader(iContainer *Owner);
 	virtual ~iFileReader();

	virtual FileSize Size() const = 0;
	virtual const char* GetFileData() const = 0;
	virtual const string& FileName() const = 0;

	iContainer *GetOwner() const { return m_Owner; }
private: 
	iContainer *m_Owner;
};

} //namespace FileSystem 
} //namespace MoonGlare 

#endif
