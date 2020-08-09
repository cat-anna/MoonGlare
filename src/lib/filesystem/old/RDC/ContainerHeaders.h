#pragma once

namespace MoonGlare {
namespace FileSystem {
namespace MoonGlareContainer {

struct FileNode : public RawFilePointer {
	string Name;
	struct {
		char Directory;
		char MetaFile;
	} Flags;

	Headers::CurrentVersion::DataBlock DataBlock;

	FileNode *Parent = nullptr;

	FileNode() {
		Flags.Directory = 0;
		Flags.MetaFile = 0;
	}

	string Path() const {
		std::stack <const FileNode*> s;
		auto *p = this;
		while (p) {
			s.push(p);
			p = p->Parent;
		}
		std::ostringstream ss;
		while (!s.empty()) {
			auto p = s.top();
			s.pop();
			ss << p->Name;
			if (p->Flags.Directory)
				ss << "/";
		}
		return ss.str();
	}
};

using FileNodeTable = std::vector < FileNode >;
using FileNodeList = std::list < FileNode >;

using StringTable = std::unique_ptr < char[] >;

struct DynamicStringTable {
	using Length = unsigned __int32;


	template<class T>
	void AddString(T&& str) {
		m_list.emplace_back(str);
		//char buf[128];
		//sprintf(buf, "%8x - %s\n", m_ByteSize, m_list.back().c_str());
		//std::cerr << buf << std::flush;
		m_ByteSize += static_cast<u32>(m_list.back().length()) + 1;
	}


	Length CurrentLocation() {
		return m_ByteSize;
	}

	unsigned __int32 ByteSize() const { 
		Length size = m_ByteSize;
		size += 1;
		if (size & 0xF) {
			size += 0x10;
			size &= ~0xF;
		}
		return size;
	}

	std::unique_ptr<Headers::Version_0::StringTableSection> GetBytes() {
		std::unique_ptr<Headers::Version_0::StringTableSection> t;
		Length size = m_ByteSize;
		size += 1;
		if (size & 0xF) {
			size += 0x10;
			size &= ~0xF;
		}
		t.reset((Headers::Version_0::StringTableSection*)new char[size]);
		t->Table[m_ByteSize] = 0;
		for (auto i = m_ByteSize + 1; i < size; ++i)
			t->Table[i] = 'X';
		char *out = t->Table;
		for (auto &it : m_list) {
			size_t len = it.length() + 1;
			memcpy(out, it.c_str(), len);
			out += len;
		}
		return t;
	}
private:
	Length m_ByteSize = 0;
	std::list<string> m_list;
};

} //namespace MoonGlareContainer 
} //namespace FileSystem 
} //namespace MoonGlare 
