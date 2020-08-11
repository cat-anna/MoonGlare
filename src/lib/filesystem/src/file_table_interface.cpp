#include "svfs/file_table_interface.h"

namespace MoonGlare::StarVfs {

#if 0

//-------------------------------------------------------------------------------------------------

FileID FileTableInterface::ForceAllocFileID(const CString InternalFullPath) {
	CString it = InternalFullPath;
	const CString base = it;
	if (*it == '/')
		++it;

	while (true) {
		size_t len;
		auto pos = strchr(it, '/');
		if (!pos) {
			len = strlen(InternalFullPath);
		} else {
			len = pos - base;
		}
		auto fid = m_Owner->Lookup(base, len);
		auto f = m_Owner->GetFile(fid);
		if (!f) {
			if (!pos) {
				return AllocFileID(InternalFullPath);
			} else {
				String parentPath(base, len);
				fid = AllocFileID((CString)parentPath.c_str());
				if (!CreateDirectory(fid, 0)) {
					STARVFSErrorLog("Failed to create directory");
					return false;
				}
				f = m_Owner->GetFile(fid);
			}
		}
		
		if (!f->m_Flags.ValidDirectory()) {
			STARVFSErrorLog("Cannot add child into file!");
			return false;
		}
		
		it = pos + 1;
	}
}

FileID FileTableInterface::AllocFileID(const CString InternalFullPath) {
	auto f = m_Owner->AllocFile(InternalFullPath);
	if (!f)
		return 0;
	f->m_ContainerFileID = 0;
	f->m_ContainerID = m_CID;
	return f->m_GlobalFileID;
}

FileID FileTableInterface::AllocFileID(FileID Parent, FilePathHash PathHash, const CString FileName) {
	auto f = m_Owner->AllocFile(Parent, PathHash, FileName);
	if (!f)
		return 0;
	f->m_ContainerFileID = 0;
	f->m_ContainerID = m_CID;
	return f->m_GlobalFileID;
}

//-------------------------------------------------------------------------------------------------

bool FileTableInterface::CreateFile(FileID fid, FileID cfid, FileSize Size) const {
	auto f = m_Owner->GetRawFile(fid);
	if (!f)
		return false;

	if (f->m_Flags.ValidDirectory()) {
		//release subtree
		assert(false);
	}

	f->m_ContainerFileID = cfid;
	f->m_Size = Size;
	f->m_Flags.intval = 0;
	f->m_Flags.Valid = 1;
    f->m_Flags.Deleted = false;
	return true;
}

bool FileTableInterface::CreateDirectory(FileID fid, FileID cfid) const {
	auto f = m_Owner->GetRawFile(fid);
	if (!f)
		return false;

	if (f->m_Flags.ValidFile()) {
		assert(false);
	}

	f->m_ContainerFileID = cfid;
	f->m_Size = 0;
	f->m_Flags.intval = 0;
	f->m_Flags.Valid = 1;
	f->m_Flags.Directory = 1;
	return true;
}

bool FileTableInterface::DeleteFile(FileID fid) {
	return m_Owner->DeleteFile(fid, m_CID);
}

//-------------------------------------------------------------------------------------------------

bool FileTableInterface::IsFileValid(FileID fid) const { 
	return m_Owner->IsValid(fid);
}

bool FileTableInterface::IsDirectory(FileID fid) const { 
	return m_Owner->IsDirectory(fid); 
}

bool FileTableInterface::IsFile(FileID fid) const { 
	return m_Owner->IsFile(fid); 
}

//-------------------------------------------------------------------------------------------------

bool FileTableInterface::RegisterFileStructure(FileID Parent, const FileSubStructureInfo& SubStructure) {
	auto parent = m_Owner->GetFile(Parent);
	if (!parent) {
		STARVFSErrorLog("Invalid parent id!");
		return false;
	}

	auto root = m_Owner->GetRoot();
//	bool UseLocalHash;
	if (root == parent) {
//		UseLocalHash = true;
	} else {
//		UseLocalHash = false;
		StarVFSAssert(false);
	}

	FileTable::FileStructureInfo info;

	info.m_Parent = parent;
	info.m_Count = SubStructure.m_Count;
	info.m_FileTable = SubStructure.m_FileTable;
	info.m_PathHashTable = SubStructure.m_LocalPathHashTable;
	info.m_OwnerContainer = GetContainerID();

	return m_Owner->RegisterStructureTable(info);
}

//-------------------------------------------------------------------------------------------------

bool FileTableInterface::UpdateFileSize(FileID fid, FileSize NewSize) {
	auto f = m_Owner->GetFile(fid);
	if (!f || !f->m_Flags.ValidFile() || f->m_ContainerID != GetContainerID())
		return false;

	f->m_Size = NewSize;

	return true;
}

#endif

} // namespace MoonGlare::StarVfs
