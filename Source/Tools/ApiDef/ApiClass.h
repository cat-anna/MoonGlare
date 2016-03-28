#ifndef ApiClassH
#define ApiClassH

#include "ApiNamespace.h"

typedef std::vector<TypePointer> LegacyVector;

class ApiClass : public ApiNamespace {
protected:
	LegacyVector m_Legacy;
public:
	ApiClass(const std::string &Name, ApiBaseType *Parent);

	virtual void LoadFromXML(const pugi::xml_node Node);
	virtual void SaveToXML(pugi::xml_node Node) const ;
	
	virtual std::string ToString(unsigned Mode) const;
	virtual const ApiBaseType* FindType(const std::string &name, unsigned Flags = 0) const;
	virtual bool Search(const std::string &Path, ConstNamespaceVector &Result, unsigned Flags = 0) const;
};

#endif
