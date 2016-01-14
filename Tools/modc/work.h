#pragma once

#include "verbosity.h"
#include "error.h"

namespace modc {

using StringStringMap = std::map < string, string > ;

struct SettingsMap : public StringStringMap {
	template<class FUNC>
	void InvokeAndRemove(const string &key, FUNC func) {
		auto it = this->find(key);
		if (it == this->end()) return;
		func(it->second);
		erase(it);
	}
};

enum class RWMode {
	None, R, W, RW, 
};

struct ContainerBasis {
	RWMode RW = RWMode::None;;
	std::shared_ptr<FileSystem::iContainer> container;

	bool CanRead() { return container && (RW == RWMode::R || RW == RWMode::RW); }
	bool CanWrite() { return container && (RW == RWMode::W || RW == RWMode::RW); }
};

struct ScriptCompilerConfig {
	string ExecFile = "luac51";

	bool Validate() { return true; }
};

enum class ModuleFileType {
	Unknown,
	CompiledModule,
	RawModule,
	InternalModule,
	None,
};

struct InputFileConfig : public ContainerBasis {
	ModuleFileType Type = ModuleFileType::Unknown;
	string FileName;
	string ModuleName = "{bad_module_name}";

	bool Validate() { return Type != ModuleFileType::Unknown; }
};

struct OutputFileConfig : public ContainerBasis {
	ModuleFileType Type = ModuleFileType::Unknown;
	string FileName = "{bad_file_name}";

	bool Validate() { return Type != ModuleFileType::Unknown; }
};

struct Flags {
	enum {
		Ignore_Errors			= 0x00000010,
		//flag_Scripts_NotCompile		= 0x10000000,
		//flag_Scripts_AddDebug			= 0x20000000,
		//flag_Dump_Content             = 0x01000000,
		//flag_dump_file				= 0x02000000,
		//flag_dump_all_to_folder		= 0x04000000,
		//flag_read_module            	= 0x00000001,
		//flag_output_specified			= 0x00000002,
		//flag_source_specified			= 0x00000004,
	};
};

struct ValidationConfig {
	string PatternFile, ClassesFile;

	ValidationConfig() {
		PatternFile = "Validation.xml";
		ClassesFile = "Classes.xml";
	}
};

enum class FileSource {
	Unknown,
	FileSystem,
	Container,
};

struct FileInfo {
	FileSource Source = FileSource::Unknown;

	string Name = "{badname}";
	string VirtualPath = "{badvpath}";

	std::shared_ptr<FileSystem::iContainer> SrcContainer;
	string FileSystemPath = "{badsyspath}";

	bool Injected = false;
	bool Ignore = false;
	bool Validated = false;
	bool Needed = false;
	bool Correct = false;
	bool Tested = false;
	bool Preloaded = false;
	bool Generated = false;

	void PreloadData() {
		if (Preloaded) return;
		switch (Source) {
		case FileSource::Container: {
			string fullname = VirtualPath + "/" + Name;
			auto file = SrcContainer->GetFileReader(fullname);
			if (!file)
				throw "File does not exists!";
			auto size = file->Size();
			PreloadedData.reset(new char[size + 1]);
			PreloadedData.get()[size] = 0;
			PreloadedSize = size;
			memcpy(PreloadedData.get(), file->GetFileData(), size);
			Preloaded = true;
			return;
		}
		default:
			throw "Wrong file source!";
		}
	}

	void ClearPreloaded() {
		if (Generated) return;
		Preloaded = false;
		PreloadedData.reset();
	}

	__int32 PreloadedSize = -1;
	std::shared_ptr<char> PreloadedData;
};

struct ContentInfoOutput {
	unsigned Flags = 0;

	struct Flags {
		enum {
				DoNotGenerateXML		= 0x0001,
		};
	};

	DefineFlagGetter(Flags, Flags::DoNotGenerateXML, DoNotGenerateXML);
};

using FileInfoList = std::list < FileInfo > ;

namespace pipeline {
	struct Pipeline;
}

struct WorkConfiguration {
	InputFileConfig input;
	OutputFileConfig output;

	FileInfoList FileList;

	ContentInfoOutput ContentOut;
	ScriptCompilerConfig Scripts;
	ValidationConfig validation;

	unsigned Flags;

	pipeline::Pipeline *pipeline;

	WorkConfiguration() {
		pipeline = 0;
	}

	bool Validate() { 
		int r = 0;
		if (!input.Validate()) ++r;
		if (!output.Validate()) ++r;
		if (!Scripts.Validate()) ++r;
		return r == 0;
	}

	DefineFlagGetter(Flags, Flags::Ignore_Errors, IgnoreErrors);

	WorkConfiguration* operator->() { return this; }

	unsigned StageIndex = 0;
	unsigned StageActionIndex = 0;
	string StageCounter() {
		if (!Verbosity::PrintStageCounter()) return "";
		char buf[64];
		sprintf(buf, "[%2d:%-3d] ", StageIndex, StageActionIndex);
		std::cout << std::flush;
		return buf;
	}
};

}// namespace modc

int backstrcmpi(const char *c1, const char *c2);
