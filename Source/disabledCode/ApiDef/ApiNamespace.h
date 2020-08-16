#ifndef ApiNamespaceH
#define ApiNamespaceH

#include "ApiBaseType.h"

class ApiNamespace : public ApiBaseType {
protected:
	NamespaceVector m_List;
	ApiNamespace(const std::string &Name, ApiBaseType *Parent, eSubclassType NodeType);
public:
	ApiNamespace(const std::string &Name, ApiBaseType *Parent);
	virtual ~ApiNamespace();

	ApiNamespace *NewNamespace(const std::string &Name);
	ApiClass *NewClass(const std::string &Name);
	ApiVariable *NewVariable(const std::string &Name);
	ApiFunction *NewFunction(const std::string &Name);
	ApiConstant *NewConstant(const std::string &Name);
	ApiType *NewType(const std::string &Name);

	unsigned GetCount() const;
	const ApiBaseType *GetItem(unsigned index) const ;

	virtual void LoadFromXML(const pugi::xml_node Node);
	virtual void SaveToXML(pugi::xml_node Node) const ;
	
	void RemoveMember(unsigned index);
	void RemoveMemberByType(eSubclassType type);
	void ClearMembers();
	
	virtual std::string ToString(unsigned Mode) const;
	virtual const ApiBaseType* FindType(const std::string &name, unsigned Flags = 0) const;
	virtual bool Search(const std::string &Path, ConstNamespaceVector &Result, unsigned Flags = 0) const;
};

#endif
