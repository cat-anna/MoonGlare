#include <iostream>
#include <cassert>
#include <functional>

#include "ApiBaseType.h"

using namespace std;

void PrintLevel(int l){
	while(l-- > 0)
		cout << "\t";
}

void ProcessNamespace(const ApiNamespace *api, int level = 0){
	assert(api);
	PrintLevel(level);
	cout << api->ToString(scmBegining) << endl;
	for(int i = 0, j = api->GetCount(); i < j; ++i){
		const ApiBaseType *a = api->GetItem(i);
		switch (a->GetNodeType()){
		case stNamespace:
		case stClass:
		 case stHeader:
			ProcessNamespace(static_cast<const ApiNamespace*>(a), level + 1);
			continue;
			
		case stVariable:	
		case stFunction:
			PrintLevel(level + 1);
			cout << a->ToString(scmLine) << endl;	
			continue;
			
		default:
			continue;
		}
	}
	PrintLevel(level);
	cout << api->ToString(scmEnding) << endl;	
}

void PrintNamespaceVector(const ConstNamespaceVector &vec, eStringConversionMode mode){
	for(int i = 0, j = vec.size(); i < j; ++i){
		const ApiBaseType *a = vec[i];
		cout << a->ToString(mode) << endl;	
	}
}

int PrintBuiltIn(ApiDefinition &defs){
	for(int i = 0, j = defs.BuiltInCount(); i < j; ++i){
		const ApiBaseType *t = defs.GetBuiltIn(i);
		cout << t->ToString(scmLine) << endl;
	}
	return 0;
}

int PrintLocals(ApiDefinition &defs){
	for(int i = 0, j = defs.LocalsCount(); i < j; ++i){
		const ApiBaseType *t = defs.GetLocal(i);
		cout << t->ToString(scmLine) << endl;
	}
	return 0;
}

int PrintTree(ApiDefinition &defs){
	for(int i = 0, j = defs.GetHeaderCount(); i < j; ++i){
		const ApiHeader *api = defs.GetHeader(i);
		ProcessNamespace(api);
		cout << endl;
	}
	return 0;
}

int Find(ApiDefinition &defs, const string &what, eStringConversionMode mode = scmNameWithPath) {
	ConstNamespaceVector vec;
	defs.Search(what, vec);
	PrintNamespaceVector(vec, mode);
	return 0;
}

int Type(ApiDefinition &defs, const string &what){
	const ApiBaseType *t = defs.FindType(what);
	if(!t) return 1;
	cout << t->ToString(scmNameWithPath) << endl;
	return 0;
}

int Func(ApiDefinition &defs, const string &what){
	/*for(int i = 0, j = defs.GetHeaderCount(); i < j; ++i){
		const ApiNamespace *hdr = defs.GetHeader(i);
		ConstNamespaceVector vec;
		if(hdr->Search(what, vec)) 
			PrintNamespaceVector(vec);
	}*/
	return 0;
}

class eNotEnoughParams {
public:
	unsigned Value;
	eNotEnoughParams(unsigned val): Value(val) {}
};

class eExit {};
class eUnknownCommand {};

typedef std::list<string> StringList;

void PrintHelp();

struct CommandParams {
	CommandParams() = delete;
	CommandParams& operator=(const CommandParams&) = delete;
	StringList &params;
	ApiDefinition &defs;

	const string &operator[](int idx) const {
		auto it = params.begin();
		std::advance(it, idx);
		return *it;
	}
};

struct Command {
	const char *Name;
	unsigned ParamCount;

	std::function<void(CommandParams&)> Function;

	const char *Description;
};


const Command CommandTable[] = {
	{ "help", 0, [](CommandParams&) { PrintHelp(); }, "Prints this help", },
	{ "exit", 0, [](CommandParams&) { throw eExit(); }, "Exit application", },

	{ "builtin", 0, [](CommandParams&p) { PrintBuiltIn(p.defs); }, "Print builtin definitions", },
	{ "locals", 0, [](CommandParams&p) { PrintLocals(p.defs); }, "Print local variables", },
	{ "list", 0, [](CommandParams&p) { PrintTree(p.defs); }, "List all external types", },

	{ "load", 1, [](CommandParams&p) { p.defs.Load(p[0]); }, "Load external ApiDefs xml", },
	{ "find", 1, [](CommandParams&p) { Find(p.defs, p[0]); }, "Look for content of object", },
	{ "type", 1, [](CommandParams&p) { Type(p.defs, p[0]); }, "Look for definition", },
	{ "disable", 1, [](CommandParams&p) { p.defs.SetHeaderFlags(p[0], ffDisableType, true); }, "Disable header", },
	{ "enable", 1, [](CommandParams&p) { p.defs.SetHeaderFlags(p[0], ffDisableType, false); }, "Enable header", },

	{ "local", 2, [](CommandParams&p) { p.defs.AddLocal(p[1], p[0]); }, "Add local variable. Syntax: Type Name", },

	//{ "exit", 0, [](CommandParams&p) { throw eExit(); }, "Exit application", },
	//{ "exit", 0, [](CommandParams&p) { throw eExit(); }, "Exit application", },
	//{ "exit", 0, [](CommandParams&p) { throw eExit(); }, "Exit application", },

	{ 0 },
};

struct CmdMap_t : public std::unordered_map<string, const Command*> {
	CmdMap_t() {
		for (const Command *cmd = CommandTable; cmd->Name; ++cmd)
			insert(std::make_pair(cmd->Name, cmd));
	}
};
static const CmdMap_t CmdMap;

int ProcessCommand(ApiDefinition &defs, StringList &input){
	if (input.empty()) return 0;
	string s = std::move(input.front());
	input.pop_front();

	auto cmdit = CmdMap.find(s);
	if (cmdit == CmdMap.end())
		throw eUnknownCommand();

	auto *cmd = cmdit->second;
	if(cmd->ParamCount > input.size())
		throw eNotEnoughParams(cmd->ParamCount - input.size());

	if(!cmd->Function) return 0;

	CommandParams params{ input, defs, };
	cmd->Function(params);

	for(unsigned i = 0; i < cmd->ParamCount; ++i)
		input.pop_front();

	return ProcessCommand(defs, input);
}

void PrintHelp() {
	cout << "Available commands:\n";
	char buf[128];
	for (const Command *cmd = CommandTable; cmd->Name; ++cmd) {
		sprintf(buf, "\t[%2d] %-15s %s\n", cmd->ParamCount, cmd->Name, cmd->Description);
		cout << buf;
	}
}

int main(int argc, char **argv){

	ApiDefinition defs;
	try{
		try{
			StringList sv;
			for (int i = 1; i < argc; ++i)
				sv.push_back(argv[i]);
			
			ProcessCommand(defs, sv);
		}
		catch(eNotEnoughParams){}
		catch(eUnknownCommand){ 
			cout << "Unknown command" << endl;
		}

		for (;;)
			try {
				cout << "> ";
				string line;
				getline(cin, line);
				stringstream ss;
				ss.str(line);
				StringList sv;
				while (!ss.eof()) {
					string s;
					ss >> s;
					sv.push_back(s);
				}
				ProcessCommand(defs, sv);
			}
			catch(eNotEnoughParams &){
				cout << "Not Enough parameters or unknown command" << endl;
			}

			catch(eUnknownCommand){
				cout << "Unknown command" << endl; 
			}
	}
	catch(eExit) { }

	return 0;
};