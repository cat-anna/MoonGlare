#ifndef ApiFunctionH
#define ApiFunctionH

#include "ApiBaseType.h"

struct sFunctionParameter {
	std::string Name;
	TypePointer Type;
	
	sFunctionParameter(){};
};

typedef std::vector<sFunctionParameter> ParamVector;

class ApiFunction : public ApiBaseType {
protected:
	ParamVector m_Params;
	TypePointer m_ResultType;
public:
	ApiFunction(const std::string &Name, ApiBaseType *Parent);

	unsigned GetParamCount() const;
	const sFunctionParameter& GetParameter(unsigned index) const;

	virtual void LoadFromXML(const pugi::xml_node Node);
	virtual void SaveToXML(pugi::xml_node Node) const ;

	virtual std::string ToString(unsigned Mode) const;
	virtual bool Search(const std::string &Path, ConstNamespaceVector &Result, unsigned Flags = 0) const;
};

#endif
