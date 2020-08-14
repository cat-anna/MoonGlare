#include <core/nStarVFS.h>
#include "../common/LogVirtualSink.h"
#include "../common/RDCMetaModule.h"
#include <core/Container/RDCContainer.h>
#include <fstream>
#include <Windows.h>
#include "wcxhead.h"

#ifdef X32
#	pragma comment(linker, "/EXPORT:OpenArchive=_OpenArchive@4")
#	pragma comment(linker, "/EXPORT:CloseArchive=_CloseArchive@4")
#	pragma comment(linker, "/EXPORT:ReadHeader=_ReadHeader@8")
#	pragma comment(linker, "/EXPORT:ProcessFile=_ProcessFile@16")
#	pragma comment(linker, "/EXPORT:SetChangeVolProc=_SetChangeVolProc@8")
#	pragma comment(linker, "/EXPORT:SetProcessDataProc=_SetProcessDataProc@8")
#else
#endif

struct rdc_handle {
	enum class MagicEnum : unsigned __int32 {
		Header = 0xC0FFE00,
	};
	MagicEnum MagicHeader = MagicEnum::Header;

	tProcessDataProc ProcessProc;
	tChangeVolProc ChangeVolProc;

	std::unique_ptr<StarVFS::StarVFS> m_SVFS;
	StarVFS::Containers::RDCContainer *m_Container = nullptr;

	StarVFS::FileID m_TraverseIterator;

	rdc_handle() {
		m_SVFS = std::make_unique<StarVFS::StarVFS>();
		auto logm = m_SVFS->AddModule<LogVirtualSink>();
		if (logm)
			logm->Enable();
	}
	 
	void CallProcessProc(char *FileName, int Size) {
		if (ProcessProc)
			ProcessProc(FileName, Size);
	}

	static rdc_handle* cast(HANDLE hh) {
		auto *h = static_cast<rdc_handle*>(hh);
		if (!h) return nullptr;
		if (h->MagicHeader != MagicEnum::Header) return nullptr;
		return h;
	}
};

#define TC_API extern "C" __declspec(dllexport)  

TC_API HANDLE __stdcall OpenArchive(tOpenArchiveData *ArchiveData) {
	rdc_handle *h = new rdc_handle();

	auto ret = h->m_SVFS->CreateContainer<StarVFS::Containers::RDCContainer>("/", ArchiveData->ArcName);

	if (ret.first != StarVFS::VFSErrorCode::Success) {
		STARVFSErrorLog("Failed to open container. Error code: %d\n", ret.first);
	} 

	h->m_Container = (StarVFS::Containers::RDCContainer*)ret.second;

	auto metam = h->m_SVFS->AddModule<RDCMetaModule>(h->m_Container);
	if (metam)
		metam->Enable();

	ArchiveData->OpenResult = 0;
	h->m_TraverseIterator = 2;

	return (HANDLE)h;
}

TC_API int __stdcall ReadHeader(HANDLE hArcData, tHeaderData *HeaderData) {
	auto *h = rdc_handle::cast(hArcData);
	if (!h) return E_BAD_ARCHIVE;

	auto svfs = h->m_SVFS.get();

	if (!svfs->IsFileValid(h->m_TraverseIterator)) {
		return E_END_ARCHIVE;
	}

	auto fid = h->m_TraverseIterator;
	++h->m_TraverseIterator;

	auto path = svfs->GetFullFilePath(fid);

	for (auto &it : path)
		if (it == '/')
			it = '\\';

	HeaderData->ArcName[0] = 0;
	strcpy_s(HeaderData->FileName, path.c_str() + 1);
	HeaderData->PackSize = 0;// svfs->GetFileSize(fid);
	HeaderData->UnpSize = svfs->GetFileSize(fid);
	HeaderData->FileCRC = 0;
	HeaderData->HostOS = 0;
	HeaderData->FileTime = 0;
	HeaderData->FileAttr = 0;

	if (svfs->IsFileDirectory(fid))
		HeaderData->FileAttr |= 0x10;
	else
		HeaderData->FileAttr |= 0x01;
//	if (file->Flags.MetaFile)
//		HeaderData->FileAttr |= 0x04;
	return 0;
}

TC_API int __stdcall ProcessFile(HANDLE hArcData, int Operation, char *DestPath, char *DestName) {
	auto *h = rdc_handle::cast(hArcData);
	if (!h) return E_BAD_ARCHIVE;

	switch (Operation) {
	case PK_SKIP:
		return 0;

	case PK_EXTRACT: {
		auto svfs = h->m_SVFS.get();
		
		auto fh = svfs->OpenFile(h->m_TraverseIterator - 1, StarVFS::RWMode::R);

		auto path1 = svfs->GetFullFilePath(h->m_TraverseIterator);
		auto path2 = svfs->GetFullFilePath(h->m_TraverseIterator - 1);

		if (!fh) {
			return E_EOPEN;
		}

		StarVFS::ByteTable ct;

		if (!fh.GetFileData(ct)) {
			return E_EREAD;
		}

		std::ofstream out(DestName, std::ios::out | std::ios::binary);
		out.write(ct.get(), ct.byte_size());
		out.close();
		return 0;
	}

	case PK_TEST:
	default:
		return E_NOT_SUPPORTED;
	}
}

TC_API int __stdcall CloseArchive(HANDLE hArcData) {
	auto *h = rdc_handle::cast(hArcData);
	if (!h)
		return 1;
	delete h;
	return 0;
}

TC_API void __stdcall SetChangeVolProc(HANDLE hArcData, tChangeVolProc pChangeVolProc1) {
	auto *h = rdc_handle::cast(hArcData);
	if (!h) return;
	h->ChangeVolProc = pChangeVolProc1;
}

TC_API void __stdcall SetProcessDataProc(HANDLE hArcData, tProcessDataProc pProcessDataProc) {
	auto *h = rdc_handle::cast(hArcData);
	if (!h) return;
	h->ProcessProc = pProcessDataProc;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		break;

	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
