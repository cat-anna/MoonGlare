#ifndef ApiHeaderH
#define ApiHeaderH

#include <vector>
#include <map>
#include <string>

#include <pugixml-1.2/src/pugixml.hpp>
#include "ApiNamespace.h"

class ApiDefinition;

class ApiHeader : public ApiNamespace {
protected:
	ApiDefinition *m_Owner;
public:
	ApiHeader(ApiDefinition *Owner, const std::string &Name);
	~ApiHeader();
	
	const ApiDefinition *GetOwner() const { return m_Owner; }

	void LoadFromXML(const pugi::xml_node node);
	void SaveToXML(pugi::xml_node node) const;

	virtual std::string ToString(unsigned Mode) const;
	virtual const ApiHeader* GetRootHeader() const;
	
	virtual const ApiBaseType* FindType(const std::string &Name, unsigned Flags = 0) const;
	virtual bool Search(const std::string &Path, ConstNamespaceVector &Result, unsigned Flags = 0) const;
};

#endif
