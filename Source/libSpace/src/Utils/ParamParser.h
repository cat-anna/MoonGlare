#ifndef SPACE_UTILS_PARAMPARSER_H
#define SPACE_UTILS_PARAMPARSER_H

#include "../Configuration.h"

namespace Space {

struct ProgramParameters {
	ProgramParameters & operator=(const ProgramParameters&) = delete;

	typedef void(*ConstParameterHandler_p)(std::vector<std::string> &arglist);
	typedef void(*SingleParameterHandler_p)(std::string &arg0);
	typedef void(*NoParameterHandler_p)();
	typedef std::function<void(const std::vector<std::string> &arglist)> FunctionalHandler_t;
	
	enum {
		disable_h_as_help			= 0x00000001,
		disable_helpmsg_on_noparams	= 0x00000004,

		supress_ParamParser_info	= 0x00000010,
	};

	class eParameterError : public std::runtime_error {
	public:
		eParameterError() : std::runtime_error("eParameterError") {};
		eParameterError(const std::string &Msg) : std::runtime_error(Msg) {};
	};
	class eNotEnoughParameters : public eParameterError {};
	class eHelpPrinted : public eParameterError {};
	
	class eDuplicatedParameter : public eParameterError {
	public:
		std::string param;
		eDuplicatedParameter(char c);
		eDuplicatedParameter(const std::string& s);
	};
	
	int Parse(int argc, char **argv);
	void PrintHelp() const;

	struct Parameter {
		class ArgHandler_t {
			ArgHandler_t& operator=(const ArgHandler_t&);//=delete;
			enum {
				ahmNone,
				ahmString,
				ahmStringVector,
				ahmCFun,
				ahmNFun,
				ahmFFun,
				ahmSFun,
				ahmInt,
				ahmDouble,
			};
			const unsigned mode;
			union {
				ConstParameterHandler_p cfun;
				NoParameterHandler_p nfun;
				SingleParameterHandler_p sfun;
				std::string *str;
				std::vector<std::string> *svec;
				int *iptr;
				double *dptr;
			} u;
			FunctionalHandler_t ffun;
			mutable unsigned m_CallCount;
		public:
			ArgHandler_t(ConstParameterHandler_p cfun);
			ArgHandler_t(NoParameterHandler_p nfun);
			ArgHandler_t(FunctionalHandler_t pffun);
			ArgHandler_t(SingleParameterHandler_p psfun);
			ArgHandler_t(std::string &str); 
			ArgHandler_t(std::string *str);
			ArgHandler_t(std::vector<std::string> &vec);
			ArgHandler_t(std::vector<std::string> *vec);
			ArgHandler_t(int *value);
			ArgHandler_t(double *value);
			ArgHandler_t(int);
			ArgHandler_t();
			int operator() (std::vector<std::string> &arglist) const;
			operator bool () const { return mode != ahmNone; }
			bool CanBeCalledMultipleTimes() const { return mode != ahmString; }
			unsigned GetCallCount() const { return m_CallCount; }
		};
		char trigger;
		unsigned argumentsCount;
		unsigned Flag;
		ArgHandler_t ArgHandler;
		char* Description;
		char* Detailed_Description;
	};

	struct ExtendedParameter {
		const char *trigger;
		unsigned argumentsCount;
		unsigned Flag;
		Parameter::ArgHandler_t ArgHandler;
		char* Description;
		char* Detailed_Description;
	};
	
	unsigned Settings;
	char *AppDescription;
	Parameter::ArgHandler_t NonParameterValues;
	char* NonParameterInfo;
	const Parameter *ParameterTable;
	const ExtendedParameter *ExtendedParameterTable;
	unsigned Flags;

	char* AppExeName;

private:
};

}; //namespace Space

#endif
