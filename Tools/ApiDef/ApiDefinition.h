#ifndef ApiDefinitionH
#define ApiDefinitionH

#include <vector>
#include <map>
#include <string>

#include "ApiBaseType.h"

class ApiHeader;

typedef std::vector<std::string> StringVector;
typedef std::vector<ApiHeader*> HeaderVector;

enum eApiDefinitionFlags {
	adfDisableBuiltInTypes		= 0x0001,
};

class ApiDefinition {
protected:
	unsigned m_Flags;
	HeaderVector m_Headers;
	ApiHeader m_BuiltInTypes;
	ApiHeader m_Locals;
public:
	ApiDefinition();
	~ApiDefinition();

	void Load(const std::string &FileName);
	void Save(const std::string &FileName) const;

	unsigned GetHeaderCount() const;
	const ApiHeader *GetHeader(unsigned index) const;

	void SetHeaderFlags(const std::string &Name, unsigned FlagSet, bool State);
	int GetHeaderIndex(const std::string &Name) const;
	
	void SetFlag(unsigned FlagSet, bool State);
	bool CheckFlag(unsigned FlagSet) const { return (m_Flags & FlagSet) == FlagSet; }

	ApiHeader *NewHeader(const std::string &HeaderName);
	
	unsigned BuiltInCount() const;
	const ApiBaseType *GetBuiltIn(unsigned index) const;	
	
	void ClearLocals();
	void AddLocal(const std::string &Name, const std::string &Type);
	unsigned LocalsCount() const;
	const ApiBaseType *GetLocal(unsigned index) const;
	
	const ApiBaseType* FindType(const std::string &Name, unsigned Flags = 0) const;
	void Search(const std::string &Path, ConstNamespaceVector &Result, unsigned Flags = 0) const;	
};

#endif
