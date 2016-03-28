#ifndef ApiVariableH
#define ApiVariableH

#include "ApiNamespace.h"

class ApiVariable : public ApiBaseType {
protected:
	TypePointer m_Type;
public:
	ApiVariable(const std::string &Name, ApiBaseType *Parent);
	
	void SetType(const std::string &type) { m_Type = type; }
	const TypePointer& GetType() const { return m_Type; }

	virtual void LoadFromXML(const pugi::xml_node Node);
	virtual void SaveToXML(pugi::xml_node Node) const ;

	virtual std::string ToString(unsigned Mode) const;
	
	virtual bool Search(const std::string &Path, ConstNamespaceVector &Result, unsigned Flags = 0) const;	
};

#endif
