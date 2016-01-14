#ifndef id004B9465_D026_453B_9B495E04CC282AFE
#define id004B9465_D026_453B_9B495E04CC282AFE

#pragma warning ( disable: 4800 )
#pragma warning ( disable: 4100 )
#pragma warning ( disable: 4505 )
#pragma warning ( disable: 4996 )
#pragma warning ( disable: 4702 )

#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <cassert>

#include <pugixml-1.2/src/pugixml.hpp>

class ApiDefinition;
class ApiNamespace;
class ApiHeader;
class ApiClass;
class ApiVariable;
class ApiFunction;
class ApiConstant;

enum eSubclassType {
	stUnknown,
	stNamespace,
	stVariable,
	stConstant,
	stFunction,
	stClass,
	stDerive,
	stHeader,
	stConstructor,
	stTypePointer,
	stType,
	stMaxValue,
};

enum eStringConversionMode {
	scmNone,
	
	scmName,
	scmType,
	scmLine,
	scmPath,
	scmNameWithPath,
	
	scmBegining,
	scmEnding,

	scmConversionModeMask 	= 0xFF,
};

enum eNamespaceFlags {
	nfConstant			= 0x0100,

	nfOwnerIsClass		= 0x1000,
	
	nfMultiParam		= 0x0010,
	nfMultiRet			= 0x0020,
};

enum eFindFlags {
	ffCompareExactName		= 0x0001,
	
	ffNoFollowInheritence	= 0x0010,
	ffSkipOwnName			= 0x0020,	
	
	ffDisableType			= 0x0100,
};

#ifndef ROOTCLASSH
struct sFlagStruct {
	const char *Name;
	unsigned Flag;
	bool Default;
};
#endif

class ApiBaseType;

typedef std::vector<ApiBaseType*> NamespaceVector;
typedef std::vector<const ApiBaseType*> ConstNamespaceVector;

class ApiBaseType {
protected:
	ApiBaseType *m_Parent;
	std::string m_Name;
	eSubclassType m_NodeType;
	unsigned m_Flags;
	
	void WriteFlagPack(pugi::xml_node Node, unsigned Flags, const sFlagStruct *FlagPack) const;
	void ReadFlagPack(pugi::xml_node Node, unsigned &Flags, const sFlagStruct *FlagPack) const;	
public:
	ApiBaseType(const std::string &Name, ApiBaseType *Parent, eSubclassType NodeType);
	virtual ~ApiBaseType();

	const std::string &GetName() const { return m_Name; }
	const ApiBaseType *GetParent() const { return m_Parent; }
	
	eSubclassType GetNodeType() const { return m_NodeType; }	

	virtual void LoadFromXML(const pugi::xml_node Node) = 0;
	virtual void SaveToXML(pugi::xml_node Node) const  = 0;	
	
	void SetFlag(unsigned FlagSet, bool State);
	bool CheckFlag(unsigned FlagSet) const { return (m_Flags & FlagSet) == FlagSet; }
	
	virtual std::string ToString(unsigned Mode) const;
	virtual const ApiHeader* GetRootHeader() const;
	virtual std::string CurrentPath() const;	
	
	virtual const ApiBaseType* FindType(const std::string &name, unsigned Flags = 0) const;
	virtual bool Compare(const std::string &Path, unsigned Flags = 0) const;
	virtual bool Search(const std::string &Path, ConstNamespaceVector &Result, unsigned Flags = 0) const;
};


class cNamespaceTypeHash : public std::unordered_map<std::string, eSubclassType> {
	void Add(const char* name, eSubclassType value) {
		insert(std::make_pair(std::string(name), value));
	}
public:
	cNamespaceTypeHash();
	eSubclassType UnsafeGet(const char* name) {
		auto it = find(name);
		if (it == end())
			return stUnknown;
		return it->second;
	}
	const char* GetText(eSubclassType type) {
		for (auto &it : *this)
			if (it.second == type)
				return it.first.c_str();
		return "Unknown";
	}
};

extern cNamespaceTypeHash NamespaceSubclasses;

#include <ApiType.h>
#include <ApiTypePointer.h>

#include <ApiNamespace.h>
#include <ApiClass.h>
#include <ApiFunction.h>
#include <ApiVariable.h>
#include <ApiHeader.h>

#include <ApiDefinition.h>

#endif // header
