#include <pch.h>
#include <GabiLib/src/utils/ParamParser.cpp>

#include "Pipeline.h"

#include <modcVersion.inc>
#include <libs/InternalFileSystem/MoonGlareContainer/nMoonGlareContainer.h>
#include "InternalContainerExporter.h"

using GabiLib::StringList;
using GabiLib::ProgramParameters;

int backstrcmpi(const char *c1, const char *c2) {
	if (!c1 || !c2) return 1;
	const char *b1 = strchr(c1, '\0') - 1;
	const char *b2 = strchr(c2, '\0') - 1;
	while (b1 >= c1 && b2 >= c2) {
		if (!*b1 || !*b2) return 0;
		char i1 = *b1;
		char i2 = *b2;
		if (isalpha(i1)) i1 |= 0x20;
		if (isalpha(i2)) i2 |= 0x20;
		if (i1 == i2) {
			--b1; --b2;
			continue;
		}
		return 1;
	}
	return 0;
}

using namespace modc;

WorkConfiguration WorkConfig;

std::ostream& LoggerSink(Log::LogLineType type) {
	static std::stringstream ss;
	switch (type) {

	case Log::LogLineType::Error:
		Error::Error();
		return cout << WorkConfig.StageCounter() << " Error: ";
	case Log::LogLineType::Warning:
		Error::Warning();
		return cout << WorkConfig.StageCounter() << " Warning: ";

	case Log::LogLineType::Hint:
	case Log::LogLineType::Debug:
		if (Verbosity::PrintInfo())
			return cout << WorkConfig.StageCounter();

	case Log::LogLineType::None:
	case Log::LogLineType::Console:
	case Log::LogLineType::MaxScreenConsole:
	case Log::LogLineType::Recon:
	case Log::LogLineType::SysInfo:
	case Log::LogLineType::Performance:
	case Log::LogLineType::MaxValue:
	default:
		ss.clear();
		return ss;
	}
}

void CheckInputFile(string &file);
void CheckOutputFile(string &file);

template<unsigned Flag, unsigned *FlagSet> 
void SetFlag2() { *(FlagSet) |= Flag; }

template<unsigned Flag> void SetFlag() { WorkConfig.Flags |= Flag; }
template<unsigned Flag> void SetContentFlag() { WorkConfig.ContentOut.Flags |= Flag; }

void SetOutputNamespace(string &data) { pipeline::OutputStage::OutputConfig["namespace"] = data; }
void SetOutputName(string &data) { pipeline::OutputStage::OutputConfig["name"] = data; }

//outputs

const ProgramParameters::Parameter Parameters[] = {
	//{ 'v', 0, 0,	&SetFlag<Flags::Validate_Only>,		"Do only module validation" },
	{ 'r', 0, 0,	&SetFlag<Flags::Ignore_Errors>,		"Ignore compilation errors" },

	{ 'o', 1, 0,	&CheckOutputFile,					"Put compiled module into specified file" },
	{ 'i', 1, 0,	&CheckInputFile,					"Specify input source. May be only compiled module or 'module.xml' file" },

	//{'c', 0, flag_Scripts_NotCompile,	0,		"Do not compile scripts."},
 	//{'g', 0, flag_Scripts_AddDebug, 	0,		"Add debug data to compiled scripts. Has effect only without switch c."},
	//{'r', 1, flag_read_module,          InputFile,	"Open module"},
	//{'d', 0, flag_Dump_Content,         0,              "Dump files tree stored in container"},
	//{'l', 1, flag_source_specified,		ParamInpList,	"Specify input files list", "\tEvery entry in files list must have format 'SYSTEM_PATH@CONTAINER_PATH'.\n\tFile name may be ommited in container path if is the same as system name."},
//	{'f', 1, flag_source_specified,		ParamInpFolder,	"Specify input folder [!]."},
	//{'e', 1, flag_dump_file,			FilesToDump,	"Dump file from container to std out"},
//	{'a', 1, flag_dump_all_to_folder, 	DumpAllToFolder, "Dump all files stored in container to folder"},
    // {'', 0, f, 0, ""},
	{'\0', 0, 0, 0}, //table ends here
}; 

using VerbositySubStep = Utils::Template::CallAll < &Verbosity::SetPrintStep, &Verbosity::SetPrintSubStep > ;

const ProgramParameters::ExtendedParameter ExtendedParameters[] = {
	//{ "luac", 1, 0,					WorkConfig.luac.ExecFile,				"Set lua compiler" },
	//{ "dump-content",		0, 0,		&SetFlag<0>,	"Disable module validation. All files from source will be included in output" },
	
	//{ "no-content-gen",		0, 0,		&SetContentFlag<ContentInfoOutput::Flags::DoNotGenerateXML>,	"Do not generate xml with module content information." },

	{ "no-validation",				0, 0,	&Stages::UnsetModuleValidation,			"Do not validate module" },
	{ "no-mediatest",				0, 0,	&Stages::UnsetMediaTest,				"Do not test files correctness" },
	{ "include-all-files",			0, 0,	&Stages::UnsetRemoveUnnecessaryFiles,	"Include all files in output container", },

	//{ "print-known-classes",		0, 0,	&SetFlag<Flags::Print_Classes>, "Print known classes" },
	//{ "print-known-resources",	0, 0,	&SetFlag<Flags::Print_Resources>, "Print known resources" },
	{ "print-file-list",			0, 0,	&Dump::SetFileList,				"Print file list" },
	{ "print-all",					0, 0,	&Dump::SetAll,					"Print evrything" },

	//{ "inject",					2, 0,	&SetFlag<Flags::Verbose_full>, "Set Maximum verbosity" },

	{ "verbose-all",				0, 0,	&Verbosity::SetAll,					"Set Maximum verbosity" },
	{ "verbose-stage",				0, 0,	&Verbosity::SetPrintStep,			"Print name of current stage" },
	{ "verbose-substep",			0, 0,	&VerbositySubStep::call,			"Print name of current stage substep" },
	{ "verbose-counter",			0, 0,	&Verbosity::SetPrintStageCounter,	"Print stage conunter" },

	{ "do-not-check-scripts",		0, 0,	&Scripts::SetDoNotCheck,			"Script files WILL NOT be processes in any way" },
	{ "do-not-compile-scripts",		0, 0,	&Scripts::SetDoNotCompile,			"Scripts syntax will be checked, but they won't be compiled" },
	{ "do-not-strip-scripts",		0, 0,	&Scripts::SetDoNotStripDebugInfo,	"Debug data will not be stripped from compiled scripts" },

	{ "namespace",					1, 0,	&SetOutputNamespace,				"Set namespace of internal container. [Internal container only]" },

	{ 0, 0, 0, 0 }, //table ends here
};

GabiLib::ProgramParameters Params = {
	ProgramParameters::disable_helpmsg_on_noparams,
	"MoonGlare module compiler",
	0,
	0,
	Parameters,
	ExtendedParameters,
}; 

string GetApplicationInfoBanner() {
	string s;
	s += "Moonglare engine module compiler ";
	s += modcToolVersion.VersionStringFull();
	return std::move(s);
}
 
int main(int argc, char **argv){
	cout << GetApplicationInfoBanner() <<  "\n\n";
	std::vector<char*> px{
		"modc",
		"-i", 
		//"./test/t.rdc",
		"./modules/Internal/",
		"-o", 
		"./test/t.inc",
		"--verbose-all", 
		"--print-all",
		"--no-validation",
		"--do-not-check-scripts",
		"--namespace=test.test.1.a",
	};
	Stages::SetAll();
	try{
		try{
			if (argc > 1)
				Params.Parse(argc, argv);
			else
				Params.Parse(px.size(), &px[0]);
		}
		catch (GabiLib::ProgramParameters::eDuplicatedParameter &e){
			cerr << "Parameter '" << e.param << "' may be specified only once\n";
			throw;
		}
	}
	catch (...){
		cerr << "Work aborted!" << endl;
		return 1;
	}

	try {
		pipeline::Pipeline p;
		p.PushStage(pipeline::EntryStage::GetInfo());
		p.Execute(WorkConfig);
	}
	catch(const std::exception &e) {
		cout << e.what() << "\n";
	}

	if(Error::GetErrorCount() || Error::GetWarningCount())
		cout << "\t" << Error::GetErrorCount() << " errors\n\t" << Error::GetWarningCount() << " warnings \n";

	return 0;
#if 0
	cModuleContainer *module = 0;

	if (Params.Flags & flag_read_module){
		cModuleContainerReader *r = new cModuleContainerReader(InputFile);
		module = r;
	} else
		if (Params.Flags & flag_output_specified){
			if (!(Params.Flags & flag_source_specified))
				cerr << "No input files specified!\n";
			else {
				cModuleContainerWritter *w = new cModuleContainerWritter();
				AddFilesToContainer(w);
				printf("Writting container...\n");
				w->SaveAs(OutputFile + ContainerHeders::ContainerEXT);
				module = w;
			}
		}

	if (!module){
		cerr << "Not enough parameters!" << endl;
		return 1;
	}

	if (Params.Flags & flag_Dump_Content){
		cout << "Dumping content:\n";
		module->DumpTree();
	}

	if (Params.Flags & flag_dump_file)
		for (auto &i : FilesToDump){
			cFileReader *f = module->GetFile(i);
			cout << f->GetFileData();
		}

	delete module;
#endif // 0
} 

void CheckInputFile(string &file) {
	WorkConfig.input.FileName = file;
	char *c = (char*)file.c_str();

	if (!backstrcmpi(c, "/module.xml") || !backstrcmpi(c, "/")) {
		WorkConfig.input.Type = ModuleFileType::RawModule;

		c = strrchr(c, '/');
		if (!c) c = strrchr(c, '\\');
		if (c) c[1] = 0;
		WorkConfig.input.FileName = file.c_str();
		return;
	}

	if (!backstrcmpi(c, RDC_EXTENSION)) {
		WorkConfig.input.Type = ModuleFileType::CompiledModule;
		return;
	}

	WorkConfig.input.Type = ModuleFileType::Unknown;
}

void CheckOutputFile(string &file) {
	WorkConfig.output.FileName = file;
	const char *c = file.c_str();

	if (!backstrcmpi(c, RDC_EXTENSION)) {
		WorkConfig.output.Type = ModuleFileType::CompiledModule;
		return;
	}

	if (!backstrcmpi(c, ".inc")) {
		WorkConfig.output.Type = ModuleFileType::InternalModule;
		return;
	}

	if (!backstrcmpi(c, "/")) {
		WorkConfig.output.Type = ModuleFileType::RawModule;
		return;
	}

	const char* filename = ::max(strrchr(c, '/'), strrchr(c, '\\'));
	const char* ext = strrchr(c, '.');
	if (ext < filename) {
		WorkConfig.output.FileName += RDC_EXTENSION;
	}

	WorkConfig.output.Type = ModuleFileType::CompiledModule;
}

#if 0
void AddFilesToContainer(cModuleContainerWritter *w){
	FilesVector input;
	for (auto it = InputSource.begin(), jt = InputSource.end(); it != jt; ++it){
		switch (it->mode){
		case 1:
			MakeFolderList(it->val1, input);
			break;
		case 2: {
			std::ifstream inpf;
			string line;
			inpf.open(it->val1, std::ios::in);
			unsigned lineno = 0;
			while (!inpf.eof()){
				getline(inpf, line);
				++lineno;
				if (line.empty()) continue;
				if (FileVecPushItem(input, line))
					cerr << "Error in files list at line " << lineno << ": Delimitier '@' not found!\n";
			}
			break;
		}
		default:
			cerr << "Unknown input mode: " << it->mode << endl;
		}
	}
	unsigned /*count = input.size(), */i = 0;
	for (auto it = input.begin(), jt = input.end(); it != jt; ++it, ++i) {
		const char *inp = it->first.c_str();
		inp = strrchr(inp, '.');
		if (inp){
			unsigned luaext = *((unsigned*)"lua");
			++inp;
			unsigned ext = *((unsigned*)inp);

			if (ext == luaext){
				CompileScript(w, it->first, it->second);
				continue;
			}

		}
		// 		printf("Adding file[%d/%d]: from %s to %s\n", i + 1, count, it->first.c_str(), it->second.c_str());
		w->AddFromFile(it->first, it->second);
	}
}

void ParamInpList(GabiLib::StringVector &arglist){
	sInputElem e;
	e.mode = 2;
	e.val1 = arglist[0];
	InputSource.push_back(e);
}

void ParamInpFolder(GabiLib::StringVector &arglist){
	sInputElem e;
	e.mode = 1;
	e.val1 = arglist[0];
	InputSource.push_back(e);
}

//	{'c', 0, flag_Scripts_NotCompile,	0,		"Do not compile scripts."},
// 	{'g', 0, flag_Scripts_AddDebug, 	0,		"Add debug data to compiled scripts. Has effect only without switch c."},

int FileVecPushItem(FilesVector &list, string listitem){
	const char* sysf = listitem.c_str();
	const char* conf = strchr(sysf, '@');
	if (!conf)
		return 1;
	*const_cast<char*>(conf) = 0;
	++conf;
	const char *n = strrchr(conf, '\\');
	if (!n) n = strrchr(conf, '/');
	if (!n) n = conf;
	else ++n;
	string temp;
	if (!*n){
		temp = conf;
		const char *sn = strrchr(sysf, '\\');
		if (!sn) sn = strrchr(sysf, '/');
		if (!sn) sn = sysf;
		else ++n;
		temp += sn;
		conf = temp.c_str();
	}
	list.push_back(make_pair(sysf, conf));
	return 0;
}

#endif // 0
