/*
  * Generated by cppsrc.sh
  * On 2015-06-10 17:04:03,22
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#include <pch.h>
#include "InternalFileSystem.h"

#include "MoonGlareContainer/nMoonGlareContainer.h"
#include "InternalContainer.h"
#include "FolderContainer.h"
#include <boost/filesystem.hpp>

namespace MoonGlare {
namespace FileSystem {

//enum class FileOpenMode {
	//Reader, XML,
//};

using UniqueInternalFilePointer = std::unique_ptr < InternalFilePointer > ;

struct DirectFilePointer {
	const RawFilePointer *FilePtr;
	iContainer *Owner;
};

struct InternalFilePointer {
	InternalFilePointer *Parent;
	DirectFilePointer CurrentPtr;

	DirectFilePointer GetFilePtr() {
		return CurrentPtr;
	}

	string Name; 
	FileSize Size; //valid if flag Used
	FileReader Reader; //valid if flag Caced

	struct {
		union {
			unsigned __intval;
			struct {
				unsigned Directory : 1;
				unsigned Shadowed : 1;

				unsigned Used : 1;
				unsigned Cached : 1;

				unsigned Redirection : 1;//not implemented
			};
		};
	} Attribs;

	bool IsFile() const { return Attribs.Directory == 0; }

	std::unordered_map<string, UniqueInternalFilePointer> Children;

	InternalFilePointer() {
		Attribs.__intval = 0;
		Parent = nullptr;
	}
};

//------------------------------------------------------------------------------------------

GABI_IMPLEMENT_STATIC_CLASS(InternalFileSystem)

InternalFileSystem::InternalFileSystem():
		m_OpenedFilesSizeLimit(0), 
		m_CurrentOpenedFilesSize(0),
		m_Flags(0) { 
	m_Root = std::make_unique < InternalFilePointer > ();
}

InternalFileSystem::~InternalFileSystem() {
}

void InternalFileSystem::OnModuleLoad(iContainer *container, unsigned LoadFlags) {
	AddLogf(Debug, "Loaded module '%s'", container->GetFileName().c_str());
}

InternalFileSystem::ContainerPrecheckStatus InternalFileSystem::OnBeforeContainerAppend(iContainer *container, unsigned LoadFlags) {
	return ContainerPrecheckStatus::Append;
}

void InternalFileSystem::StructureChangedNotification() {
	//nothing here
}

//------------------------------------------------------------------------------------------

bool InternalFileSystem::LoadModule(const string& FileName, unsigned LoadFlags) {
	auto container = OpenContainer(FileName, LoadFlags);
	if (!container) {
		AddLog(Error, "Unable to open container: " << FileName);
		return false;
	}
	return AppendModule(container, LoadFlags);
}

bool InternalFileSystem::AppendModule(Container &module, unsigned AppendFlags) {
	if (!module) {
		AddLog(Error, "Inavlid module pointer!");
		return false;
	}

	auto status = OnBeforeContainerAppend(module.get(), AppendFlags);

	switch (status) {
	case ContainerPrecheckStatus::Append:
		//silently continue
		break;
	case ContainerPrecheckStatus::Discard:
		AddLogf(Hint, "Discarding container '%s'", module->GetFileName());
		module.reset();
		return true;
	default:
		AddLog(InvalidEnum, status);
		break;
	}

	m_Containers.emplace_front(std::move(module));
	Container &c = m_Containers.front();

	ShadowFileSystem(c.get());
	AddLogf(Debug, "Appended container '%s'", c->GetFileName().c_str());

	OnModuleLoad(c.get(), AppendFlags);


	return true;
}

bool InternalFileSystem::RegisterInternalContainer(const InternalFileNode *root, InternalContainerImportPriority Priority) {
	if (IsRegisteredContainerOpen())
		return false;
	using T = decltype(this->m_InternalContainerRegister);
	if (!m_InternalContainerRegister)
		m_InternalContainerRegister = std::make_unique<T::element_type>();

	m_InternalContainerRegister->push_back(std::make_pair(root, Priority));
	return true;
}

bool InternalFileSystem::LoadRegisteredContainers(unsigned LoadFlags) {
	if (IsRegisteredContainerOpen())
		return false;
	SetRegisteredContainerOpen(true);
	if (!m_InternalContainerRegister)
		return true;
	using Tlist = decltype(this->m_InternalContainerRegister);
	using Telem = Tlist::element_type::value_type;

	m_InternalContainerRegister->sort([](const Telem &a, const Telem &b) { return a.second < b.second; });
	for (auto &it : *m_InternalContainerRegister.get()) { 
		Container c = std::make_unique<InternalContainer>(it.first);
		if (!AppendModule(c)) {
			AddLogf(Error, "Unable to append internal container %p with priority %d!", it.first, it.second);
		}
	}

	m_InternalContainerRegister.reset();
	return true;
}

//------------------------------------------------------------------------------------------

FileReader InternalFileSystem::OpenFile(const string& File) {
	auto ptr = InternalOpenFile(File);
	if (!ptr)
		return nullptr;
	return ptr->Reader;
}

FileWritter InternalFileSystem::OpenFileForWrite(const string& File) {
	auto internal_ptr = FindPointer(File);
	if (!internal_ptr || !internal_ptr->IsFile()) {
		AddLog(Warning, "Unable to find file: '" << File << "'");
		return nullptr;
	}

	auto fptr = internal_ptr->GetFilePtr();
	return fptr.Owner->GetFileWritter(fptr.FilePtr);
}

InternalFilePointer* InternalFileSystem::InternalOpenFile(const string& File) {
	auto internal_ptr = FindPointer(File);
	if (!internal_ptr || !internal_ptr->IsFile()) {
		AddLog(Warning, "Unable to find file: '" << File << "'");
		return nullptr;
	}

	auto fptr = internal_ptr->GetFilePtr();

	if (internal_ptr->Attribs.Cached) {
		AddLogf(Debug, "File '%s' opened from cache", File.c_str());
		return internal_ptr;
	}

	internal_ptr->Reader = fptr.Owner->GetFileReader(fptr.FilePtr);
	if (!internal_ptr->Reader) {
		AddLogf(Error, "Unable to open file for read: '%s'", File.c_str());
		return false;
	}
	internal_ptr->Size = internal_ptr->Reader->Size();

	PushFileCache(internal_ptr);
	
	AddLogf(Debug, "Opened file '%s' from container '%s'", File.c_str(), fptr.Owner->GetFileName().c_str());
	return internal_ptr;
}

bool InternalFileSystem::OpenXML(XML &doc, const string& File) {
	doc.reset();
	auto reader = OpenFile(File);

	if (!reader) {
		//silently ignore, error ought to be printed
		return false;
	}

	doc = std::make_unique<xml_document>();
	auto load_ret = doc->load_buffer(reader->GetFileData(), reader->Size());

	if (load_ret.status != pugi::status_ok) {
		AddLogf(Error, "XML document is invalid! Error code: %d", load_ret);
		doc.reset();
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------------------

InternalFilePointer* InternalFileSystem::FindPointer(const string &FileName) {
	if (FileName.empty())
		return nullptr;

	string fn = FileName;

#ifdef INTERNAL_FILE_SYSTEM_NO_CASE_SENSITIVE
	std::transform(fn.begin(), fn.end(), fn.begin(), ::tolower);
#endif
#ifdef INTERNAL_FILE_SYSTEM_NO_WINDOWS_PATH_SUPPORT
	std::transform(fn.begin(), fn.end(), fn.begin(), [](int c)->int { if (c == '\\') return '/'; return c; });
#endif

	InternalFilePointer **item_ptr;
	{
		LOCK_MUTEX(m_CacheLock);
		item_ptr = &m_FileCache[fn];
	}
	auto *item = *item_ptr;
	if (item) {
		//AddLog(Debug, "Ifs cache hit: " << FileName);
		return item;
	}
	//AddLog(Debug, "Ifs cache miss: " << FileName);

	{
		LOCK_MUTEX(m_Lock);
		char *path = (char*)fn.c_str();
		string name;
		item = m_Root.get();
		while (path && *path) {
			while (*path == '/') ++path;
			char *c = strchr(path, '/');
			if (c) {
				*c = 0;
				name = path;
				path = c + 1;
			} else {
				name = path;
				path = nullptr;
			}

			auto nodeit = item->Children.find(name);
			if (nodeit == item->Children.end()) {
				AddLog(Warning, "Unable to find file: " << FileName);
				return nullptr;
			} else {
				item = nodeit->second.get();
			}
		}
	}
	item->Attribs.Used = 1;
	{
		LOCK_MUTEX(m_CacheLock);
		*item_ptr = item;
	}
	return item;
}

//------------------------------------------------------------------------------------------

bool InternalFileSystem::EnumerateFolder(const string &RootFolder, FileTable &files) {
	AddLogf(Debug, "Enumerating folder '%s'", RootFolder.c_str());
	auto internal_ptr = FindPointer(RootFolder);
	if (!internal_ptr || internal_ptr->IsFile()) {
		AddLogf(Error, "Invalid RootFolder for enumeration provided ('%s')", RootFolder.c_str());
		return false;
	}

	LOCK_MUTEX(m_Lock);
	for (auto &it: internal_ptr->Children) {
		files.emplace_back(FileInfo{it.second->Name, it.second->Attribs.Directory ? FileType::Directory : FileType::File});
	}
	return true;
}

//------------------------------------------------------------------------------------------

void InternalFileSystem::PushFileCache(InternalFilePointer *internal_ptr) {
	//Cache is disabled
#if 0
	internal_ptr->Attribs.Cached = 1;
	do {
		LOCK_MUTEX(m_CacheLock);
		m_CurrentOpenedFilesSize += internal_ptr->Size;
		++m_CurrentOpenedFilesCount;
	} while (false);
	AddLogf(Debug, "File cache load: %.3fKiB (%d files)", m_CurrentOpenedFilesSize / 1024.0f, m_CurrentOpenedFilesCount.load());

#endif // 0
}

void InternalFileSystem::FlushCache() {
	LOCK_MUTEX(m_CacheLock);
	m_FileCache.clear();
}

static void ShadowDirectory(InternalFilePointer *root, const RawFilePointer *dir, iContainer *source) {

	auto func =[root, dir, source](const string &FileName, FileType type, const RawFilePointer *ptr) {
		string lowname = FileName;
#ifdef INTERNAL_FILE_SYSTEM_NO_CASE_SENSITIVE
		std::transform(lowname.begin(), lowname.end(), lowname.begin(), ::tolower);
#endif
		auto &item = root->Children[lowname];
		bool doset = true;
		if (!item) {
			item = std::make_unique<InternalFilePointer>();
			item->Parent = root;
			item->Name = lowname;
		} else {
			if (type != FileType::Directory) {
				item->Attribs.Shadowed = 1;
			} else {
				doset = false;
			}
		}
		
		if (doset) {
			item->CurrentPtr.Owner = source;
			item->CurrentPtr.FilePtr = ptr;
		}

		switch (type) {
		case FileType::File:
			item->Attribs.Directory = 0;
			break;
		case FileType::Directory:
			item->Attribs.Directory = 1;
			ShadowDirectory(item.get(), ptr, source);
			break;
		default:
			AddLog(InvalidEnum, type);
			return;
		}
	};

	if (!source->EnumerateFolder(dir, func)) {
		AddLog(Error, "Unable to enumerate container folder!");
	}
}

void InternalFileSystem::ShadowFileSystem(iContainer *source) {
	if (!source)
		return; //silently ignore
	{
		LOCK_MUTEX(m_Lock);
		LOCK_MUTEX(m_CacheLock);
		FlushCache();
		ShadowDirectory(m_Root.get(), nullptr, source);
	}
	StructureChangedNotification();
}

//------------------------------------------------------------------------------------------

void InternalFileSystem::DumpStructure(std::ostream &out) {
	static const char* linefmt = "%10s %-30s %-8s %s\n";
	{
		char buf[1024];
		sprintf(buf, linefmt, "Size[kb]", "Container", "Attr", "File name");
		out << buf;
	}
	struct __totals {
		float size;
		unsigned fcount;
		unsigned dcount;

		__totals() {
			size = 0;
			fcount = 0;
			dcount = 0;
		}
	} totals ;
	
	std::function<void(InternalFilePointer *, int)> fun_ptr;
	auto fun = [&fun_ptr, &out, &totals](InternalFilePointer *item, int level) {
		//bool dir = !item->Children.empty();
		if (level > 0) {
			char linebuf[1024];
			char sizebuf[32];
			char attrbuf[8] = { };
			char namebuf[1024];
			namebuf[0] = '\0';

			memset(attrbuf, ' ', 6);

			if (item->Attribs.Directory) {
				attrbuf[0] = 'D';
				sizebuf[0] = '\0';
				++totals.dcount;
			} else {
				float s = (float)item->CurrentPtr.FilePtr->GetFileSize();
				sprintf(sizebuf, "%.2f", s / 1024.0f);
				totals.size += s;
				++totals.fcount;
			}

			if (item->Attribs.Shadowed) 
				attrbuf[1] = 'S';
			if (item->Attribs.Used) 
				attrbuf[2] = 'U';
			if (item->Attribs.Redirection) 
				attrbuf[3] = 'R';
			if (item->Attribs.Cached) 
				attrbuf[4] = 'C';

			auto fptr = item->GetFilePtr();

			for (int i = 1; i < level; ++i)
				strcat(namebuf, "  ");
			strcat(namebuf, item->Name.c_str());
			sprintf(linebuf, linefmt, sizebuf, fptr.Owner->GetContainerName().c_str(), attrbuf, namebuf);
			out << linebuf;

		}

		for (auto &it: item->Children) {
			fun_ptr(it.second.get(), level + 1);
		}
	};
	fun_ptr = fun;
	fun(m_Root.get(), 0);

	{
		char buf[256];
		char sizebuf[64];
		totals.size /= 1024.0f;
		if (totals.size >= 1000.0f)
			sprintf(sizebuf, "%.2f MiB", totals.size / 1024.0f);
		else
			sprintf(sizebuf, "%.2f KiB", totals.size);

		sprintf(buf, "\nFiles: %d\nFolders: %d\nTotal size: %s\n", totals.fcount, totals.dcount, sizebuf);
		out << buf;
	}

	out << "\n\n";
	out << "Loaded containers:\n";
	for (auto &it : m_Containers)
		out << it->GetFileName() << "\n";
}

//------------------------------------------------------------------------------------------

Container InternalFileSystem::OpenContainer(const string& FileName, unsigned LoadFlags) {
	if (!boost::filesystem::exists(FileName)) {
		AddLogf(Error, "File '%s' does not exists!", FileName.c_str());
		return nullptr;
	}

	Container c;
	try {
		if (boost::filesystem::is_directory(FileName)) {
#ifndef _FEATURE_FOLDER_CONTAINER_SUPPORT_
			AddLogf(Error, "File '%s' is an directory!", FileName.c_str());
			throw false;
#else 
			auto *mgc = new FolderContainer(FileName);
			c.reset(mgc);
			if (c->IsReady() && c->IsReadable()) {//try open container
				return c;
			}
#endif
		}
		
		if (1) {
			AddLog(TODO, "Consider: test RDC container extension");
			auto mgc = new MoonGlareContainer::Reader(FileName);
			c.reset(mgc);
			if (c->IsReady() && c->IsReadable()) {//try open container
				return c;
			}
		}
		c.reset();
	}
	catch (...) {
	}
	AddLogf(Error, "Unable to open container for file '%s'!", FileName.c_str());
	return nullptr;
}

} //namespace FileSystem 
} //namespace MoonGlare 
