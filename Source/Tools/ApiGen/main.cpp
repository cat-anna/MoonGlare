#include <pch.h>
#include <GabiLib/src/utils/ParamParser.cpp>

#include "ApiDefAutoGen.h"

#include <MoonGlare.h>

typedef ApiDefAutoGen::Namespace AutoGenType;
using namespace std;

void WritePrimaryDefs() {
	cout << "Generating 'ApiDef.xml'" << endl;
	ApiDefAutoGen::Namespace::Initialize("ApiDef.xml");
	{
		ApiDefAutoGen::Namespace::SetRoot("Primary");
		Core::Scripts::cScript *s = new Core::Scripts::cScript();
		s->Initialize();
		s->Finalize();
		delete s;
		//cLuaApiInit::Primary(ApiDefAutoGen::Namespace::Begin(0));
		//cFPSEngine().GetScriptEngine()->InitializeGlobalVariables();
		//cLuaApiInit::Debug(ApiDefAutoGen::Namespace::Begin("Debug"));
		//cLuaApiInit::Math(ApiDefAutoGen::Namespace::Begin("Math"));
	}
	ApiDefAutoGen::Namespace::Finalize();
	cout << "Generating 'ApiOutCall.xml'" << endl;
	ApiDefAutoGen::OutCallsPicker::WriteOutCalls("ApiOutCall.xml");
}

void WriteModuleApi(GabiLib::StringVector &arglist){
	try {	
		throw "Function not available";
#if 0
		cDataModule *Module = new cDataModule(0);
		cout << "Loading: '" << arglist[0] << "'\t";
		Module->Load(arglist[0]);
		string hdr = Module->GetModuleName();
		string file = hdr + "Api.xml";
		cout << "Generating '" << file << "'" << endl;
		ApiDefAutoGen::Namespace::Initialize(file.c_str());
		ApiDefAutoGen::Namespace::SetRoot(hdr.c_str());
		Module->GetGameEngine()->InitializeScripts(ApiDefAutoGen::Namespace::Begin(0));
		ApiDefAutoGen::Namespace::Finalize();
		delete Module;
#endif // 0
	}
	catch (const char * Msg){
		cerr << "FATAL ERROR! '" << Msg << "'" << endl;
	}
	catch (string & Msg){
		cerr << "FATAL ERROR! '" << Msg << "'" << endl;
	}
	catch (std::exception &E){
		cerr << "FATAL ERROR! '" << E.what() << "'" << endl;
	}
	catch (...){
		cerr << "UNKNOWN FATAL ERROR!" << endl;
	}
}

void WriteClassLists() {
	cout << "Generating 'Classes.xml'" << endl;
	xml_document doc;
	xml_node root = doc.append_child("Classes");
	Core::Interfaces::Initialize();
	auto processList = [&root](Core::Interfaces::ClassLists listID, const string& ListName) {
		xml_node list = root.append_child("List");
		list.append_attribute("Name") = ListName.c_str();
		list.append_attribute("Id") = (unsigned)listID;
		auto processClass = [&list](const Core::Interfaces::ClassInfo& info) {
			xml_node cl = list.append_child("Class");
			cl.append_attribute("Name") = info.Name.c_str();
			cl.append_attribute("ClassName") = info.Class->Name;
			if (info.Class->Base)
				cl.append_attribute("BaseClass") = info.Class->Base->Name;
		};
		Core::Interfaces::EnumerateClassListContent(listID, processClass);
	};

	Core::Interfaces::EnumerateClassLists(processList);
	doc.save_file("Classes.xml");
}
 
void WriteAllStatic() {
	WritePrimaryDefs();
	WriteClassLists();
}

using GabiLib::ProgramParameters; 
const ProgramParameters::Parameter Parameters[] = {
	{'p', 0, 0,		WritePrimaryDefs,	"Write primary definitions." },
	//{'m', 1, 0,		WriteModuleApi,		"Write definitions for specified module." },

	{'c', 0, 0,		WriteClassLists,	"Write class lists content." },

	{'a', 0, 0,		WriteAllStatic, "Write all static definitions."},

	{0}, // end of table marker
}; 

ProgramParameters Params = {
	0,
	"FPSEngine ApiDef generator",
	0,
	0,
	Parameters,
	0,
}; 

int main(int argc, char **argv){
	try{
		try{		
			Params.Parse(argc, argv);
		} catch (runtime_error &e){
			cerr << e.what() << endl;
			throw;
		}
	} catch(...){
		cerr << "Work aborted!" << endl;
		return 1;
	}
	return 0;
}
