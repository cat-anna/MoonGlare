#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "ParamParser.h"

namespace Space {

static std::string dupeMsg(const std::string& c){
	char buf[1024] = {};
	sprintf(buf, "Parameter '%s' may be specified only once.", c.c_str());
	return buf;
}

ProgramParameters::eDuplicatedParameter::eDuplicatedParameter(char c): 
		eParameterError(dupeMsg(std::string(1, c))), param(std::string(1, c)) { }

ProgramParameters::eDuplicatedParameter::eDuplicatedParameter(const std::string& s) :
		eParameterError(dupeMsg(s)), param(s) { }
		
template <class P>
void TriggerHandler(const P& p, std::list<std::string>& input) {
	std::vector<std::string> vec(p.argumentsCount);

	if (input.size() < p.argumentsCount){
		std::cerr << "Not enough parameters for switch '" << p.trigger << "'!" << std::endl;
		throw ProgramParameters::eNotEnoughParameters();
	}

	for (unsigned i = 0; i < p.argumentsCount; ++i){
		vec[i].swap(input.front());
		input.pop_front();
	}
	switch (p.ArgHandler(vec)){
	case -1:
		throw ProgramParameters::eDuplicatedParameter(p.trigger);
	}
}

inline int strcmp_ex(const char *c1, const char* c2, char delim) {
	while (*c1 && *c2) {
		if (*c1 == *c2) {
			++c1; ++c2; continue;
		}
		if (*c1 == delim) return 0;
		if (*c2 == delim) return 0;
		return 1;
	}
	return 0;
}

int ProgramParameters::Parse(int argc, char **argv){
	AppExeName = argv[0];
	if(argc == 1){
		if(!(Settings & disable_helpmsg_on_noparams))
			PrintHelp();
		return 0;
	}
	
	if(!ParameterTable) return 0;
	
	std::list<std::string> params;
	for(int i = 1; i < argc; ++i)
		params.push_back(argv[i]);
	
	bool hHelpEnabled = !(Settings & disable_h_as_help);
	bool helpHelpEnabled = true;// !(Settings & disable_h_as_help);
	bool hHelpPrited = false;
	
	while(!params.empty()){
		std::string cstr;
		cstr.swap(params.front());
		params.pop_front();
		const char *c = cstr.c_str();
		if(*c != '-'){
			if(NonParameterValues){
				std::vector<std::string> sv;
				sv.push_back(c);
				NonParameterValues(sv);
			}
			else std::cerr << "Unexpected parameter, value: '" << c << "'" << std::endl;
			continue;
		}
		c++;
		if (*c == '-') {//extended table
			++c;
			if (helpHelpEnabled && !strcmp(c, "help")) {
				if (!hHelpPrited) PrintHelp();
				hHelpPrited = true;//to prevent second help be printed
				continue;
			}
			for (const ExtendedParameter *p = ExtendedParameterTable; *c;) {
				if (!p->trigger){
					std::cerr << "Unknown switch '" << c << "'" << std::endl;
					break;
				}
				if (strcmp_ex(c, p->trigger, '=')) {
					++p;
					continue;
				}
				Flags |= p->Flag;
				if (p->ArgHandler){
					if (p->argumentsCount > 0) {
						const char *p0 = strchr(c, '=');
						if (p0) {
							c = p0 + 1;
							params.push_front(c);
						}
					}
					TriggerHandler(*p, params);
					break;
				}
			}
			continue;
		}
		for(const Parameter *p = ParameterTable; *c; ){
			if(hHelpEnabled && *c == 'h'){
				if(!hHelpPrited)
					PrintHelp();
				hHelpPrited = true;//to prevent second help be printed
				++c;
				continue;
			}
			if(p->trigger == 0){
				std::cerr << "Unknown switch '" << *c++ << "'" << std::endl;
				p = ParameterTable;	
				continue;
			}
			if(p->trigger != *c){
				++p;
				continue;
			}
			Flags |= p->Flag;
			if(*c) c++;			
			if(p->ArgHandler){
				if (p->argumentsCount == 1 && *c) params.push_front(c);
				TriggerHandler(*p, params);
			}
			p = ParameterTable;
		}
	}
	if(hHelpPrited) 
		throw eHelpPrinted();
	return 0;
}

void ProgramParameters::PrintHelp() const {
	const char* exen = strrchr(AppExeName, '\\');
	if(!exen) exen = strrchr(AppExeName, '/');
	if(!exen) exen = AppExeName;
	else ++exen;
	printf(
		"%s - %s\n"
		"Usage:\n"
		"\t%s [OPTIONS] %s\n\n"
		"Options:\n",
		exen, AppDescription, exen, (NonParameterValues?NonParameterInfo:""));
	bool HasDetails = false;

	if (ParameterTable) {
		for(const Parameter *p = ParameterTable; p->trigger != 0; ++p){
			char buf[512] = {};
			char buf2[512] = {};
			bool br = p->argumentsCount || p->Detailed_Description;
			if(br) strcat(buf, "[");
			if(p->argumentsCount){
				sprintf(buf2, " %d parameter%s required", p->argumentsCount, ((p->argumentsCount > 1)?"s":""));
				strcat(buf, buf2);
			}
			if(p->Detailed_Description)	strcat(buf, " See details below.");
			if(br) strcat(buf, " ]");
			if(p->Description)
				printf("     -%c\t\t%s %s\n", p->trigger, p->Description, buf);
			HasDetails |= p->Detailed_Description != 0;
		}
		if(!(Settings & disable_h_as_help))
			printf("     -h\t\tPrint this help\n");
	}
	if (ExtendedParameterTable && ParameterTable) 
		printf("\n");
	if (ExtendedParameterTable) {
		for (const ExtendedParameter *p = ExtendedParameterTable; p->trigger; ++p){
			char buf[512] ={};
			char buf2[512] ={};
			bool br = p->argumentsCount || p->Detailed_Description;
			if (br) strcat(buf, "[");
			if (p->argumentsCount){
				sprintf(buf2, " %d parameter%s required", p->argumentsCount, ((p->argumentsCount > 1) ? "s" : ""));
				strcat(buf, buf2);
			}
			if (p->Detailed_Description)	strcat(buf, " See details below.");
			if (br) strcat(buf, " ]");
			if (p->Description)
				printf("    --%-25s %s %s\n", p->trigger, p->Description, buf);
			HasDetails |= p->Detailed_Description != 0;
		}
		printf("    --%-25s %s\n", "help", "Prints this help");
	}

	if(HasDetails){
		printf("\nDetails:\n");
		for(const Parameter *p = ParameterTable; p->trigger != 0; ++p){
			if(!p->Detailed_Description) continue;
			printf("For switch -%c:\n%s\n", p->trigger, p->Detailed_Description);
		}
		for (const ExtendedParameter *p = ExtendedParameterTable; p->trigger != 0; ++p){
			if (!p->Detailed_Description) continue;
			printf("For switch --%s:\n%s\n", p->trigger, p->Detailed_Description);
		}
	}
	if(!(Settings & supress_ParamParser_info))
		printf("----- TBD -----\n");// "\nCreated by %s Parameter Parser\n", GetGabiLibInfoString());
	std::cout << std::endl << std::flush;
}

ProgramParameters::Parameter::ArgHandler_t::ArgHandler_t(ConstParameterHandler_p cfun): mode(ahmCFun), m_CallCount(0) {
	u.cfun = cfun;
}

ProgramParameters::Parameter::ArgHandler_t::ArgHandler_t(FunctionalHandler_t pffun) : mode(ahmFFun), m_CallCount(0) {
	ffun = pffun;
}

ProgramParameters::Parameter::ArgHandler_t::ArgHandler_t(NoParameterHandler_p nfun): mode(ahmNFun), m_CallCount(0) {
	u.nfun = nfun;
}

ProgramParameters::Parameter::ArgHandler_t::ArgHandler_t(SingleParameterHandler_p sfun) : mode(ahmSFun), m_CallCount(0) {
	u.sfun = sfun;
}
ProgramParameters::Parameter::ArgHandler_t::ArgHandler_t(std::string &str) : mode(ahmString), m_CallCount(0)  {
	u.str = &str;
}

ProgramParameters::Parameter::ArgHandler_t::ArgHandler_t(std::string *str) : mode(ahmString), m_CallCount(0)  {
	u.str = str;
}
  
ProgramParameters::Parameter::ArgHandler_t::ArgHandler_t(std::vector<std::string> &vec) : mode(ahmStringVector), m_CallCount(0)  {
	u.svec = &vec;
}
  
ProgramParameters::Parameter::ArgHandler_t::ArgHandler_t(std::vector<std::string> *vec) : mode(ahmStringVector), m_CallCount(0)  {
	u.svec = vec;
}

ProgramParameters::Parameter::ArgHandler_t::ArgHandler_t(int *value) : mode(ahmInt), m_CallCount(0)   {
	u.iptr = value;
} 

ProgramParameters::Parameter::ArgHandler_t::ArgHandler_t(double *value) : mode(ahmDouble), m_CallCount(0)   {
	u.dptr = value;
}

ProgramParameters::Parameter::ArgHandler_t::ArgHandler_t(int): mode(ahmNone), m_CallCount(0) { }
ProgramParameters::Parameter::ArgHandler_t::ArgHandler_t(): mode(ahmNone), m_CallCount(0) { }

int ProgramParameters::Parameter::ArgHandler_t::operator() (std::vector<std::string> &arglist) const {
	++m_CallCount;
	switch(mode){
	case ahmCFun:
		u.cfun(arglist);
		return 0;	
	case ahmFFun:
		ffun(arglist);
		return 0;
	case ahmNFun:
		u.nfun();
		return 0;
	case ahmSFun:
		u.sfun(arglist[0]);
		return 0;
	case ahmString:
		if(m_CallCount > 1) return -1;
		(*u.str) = arglist.front();
		return 0;
	case ahmStringVector:
		u.svec->push_back(arglist.front());
		return 0;
	case ahmInt:
		if(m_CallCount > 1) return -1;
		*u.iptr = strtol(arglist.front().c_str(), 0, 10);
		return 0;
	case ahmDouble:
		if(m_CallCount > 1) return -1;
		*u.dptr= strtod(arglist.front().c_str(), 0);
		return 0;		
	}
	return -2;
}

} //namespace Space
