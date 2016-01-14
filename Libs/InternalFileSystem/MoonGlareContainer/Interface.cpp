#include <pch.h>
#if 0
#include <FPSUtils.h>
#include "nContainer.h"

namespace Container {


//-------------------------------------------------------------------------------------------------
GABI_IMPLEMENT_CLASS_NOCREATOR(cContainerFile);
GABI_IMPLEMENT_CLASS_NOCREATOR(cFileWritter);
GABI_IMPLEMENT_CLASS_NOCREATOR(cFileReader);
GABI_IMPLEMENT_CLASS_NOCREATOR(ModuleContainer)

ModuleContainer::ModuleContainer(): cRootClass(), m_Flags(0), m_Version(mcVersion_Lastest), m_RootNode(0) { }

ModuleContainer::~ModuleContainer() { }

bool ModuleContainer::DeleteFile(const string& File){
	return false;
}

bool ModuleContainer::EnumerateFolder(const string &Path, std::function<void(const string&)> func) const {
	return false;
}

void ModuleContainer::DumpTree(std::ostream& out) const {
	struct T : FilesTreeRecurserConst {
		T(std::ostream& out) : count(0), size(0), aout(out) {};
		unsigned count, size;
		std::ostream& aout;
		T& operator=(const T&) { return *this; }
	protected:
		void work(const cFileNode *node){
			++count;
			size += node->Size;
			char buffer[64];
			char * where = buffer + sprintf(buffer, "%7d ", node->Size);
			for (unsigned i = 0; i < Level; ++i)
				*where++ = '\t';
			sprintf(where, "%s", node->Name);
			aout << buffer;
		}
	} t(out);
	out << "Dumping container\n";
	t.start(m_RootNode);
	char buf[64];
	sprintf(buf, "Total %.2f kbytes in %d files\n", t.size / (1024.0f), t.count);
	out << buf;
}

cFileWritter* ModuleContainer::GetFileWritter(const string &file) {
	return new cFileWritter(this);
}

unsigned ModuleContainer::FilesCount() const {
	return FileCounter::DoCount(m_RootNode);
}

bool ModuleContainer::FileExists(const string& File) const {
	return FindFile(File) != 0;
}

cFileNode* ModuleContainer::FindFile(const string& FileName) const {
	cFileNode *ptr = m_RootNode;
	
	const char* p = FileName.c_str();
	if(*p == '\\' || *p == '/') ++p;
	for (;;){
		const char* next = strchr(p, '\\');
		if(!next) next = strchr(p, '/');
		if(!next) next = p;
		try{
			std::list<cFileNode *,std::allocator<cFileNode *> >::iterator it = ptr->Children.begin();
			std::list<cFileNode *,std::allocator<cFileNode *> >::iterator jt = ptr->Children.end();
			for(; it != jt; ++it){
				cFileNode *i = *it;
				const char* s = p;
				const char* t = i->Name.c_str();
				for (;;){
					if((*s == '\\' || *s == '/' || !*s) && !*t){
						throw i;
					}
					if((*s | 0x20) != (*t | 0x20)) break;
					++s;
					++t;
				}
			}
			return 0;
		} 
		catch(cFileNode *i){
			if(p == next) return i;
			p = next + 1;
			ptr = i;
			continue;
		}
	}
}
//-------------------------------------------------------------------------------------------------
cFileNode * cFileNode::AppendChild(cFileNode *n){
	n->Parent = this;
	Children.push_back(n);
	return n;
}

cFileNode::~cFileNode(){
	for(std::list<cFileNode*>::iterator it = Children.begin(), jt = Children.end(); it != jt; ++it)
		delete *it;
}
//-------------------------------------------------------------------------------------------------
void FilesTreeRecurserConst::start(const cFileNode *ptr) {
	std::list<cFileNode *>::const_iterator it = ptr->Children.begin();
	std::list<cFileNode *>::const_iterator jt = ptr->Children.end();
	for(; it != jt; ++it){
		const cFileNode *i = *it;
		work(i);
		if(i->Flags & mcFolder){
			++Level;
			start(i);
			--Level;
		}
	}
}

void FilesTreeRecurser::start(cFileNode *ptr) {
	std::list<cFileNode *>::iterator it = ptr->Children.begin();
	std::list<cFileNode *>::iterator jt = ptr->Children.end();
	for(; it != jt; ++it){
		cFileNode *i = *it;
		work(i);
		if(i->Flags & mcFolder){
			++Level;
			start(i);
			--Level;
		}	
	}
}

FileCounter::FileCounter(): Count(0) { }

unsigned FileCounter::DoCount(const cFileNode *root) {
	FileCounter c;
	c.start(root);
	return c.Count;
}

void FileCounter::work(const cFileNode *i) {
	++Count;
}

} //namespace container
#endif
