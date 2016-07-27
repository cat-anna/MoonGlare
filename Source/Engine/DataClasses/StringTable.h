/*
  * Generated by cppsrc.sh
  * On 2015-03-08 19:20:50,34
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef StringTable_H
#define StringTable_H

namespace MoonGlare {
namespace DataClasses {

class StringTable : public cRootClass {
	SPACERTTI_DECLARE_STATIC_CLASS(StringTable, cRootClass)
public:
 	StringTable();
 	virtual ~StringTable();

	const string& GetString(const string& id, const string& Table);

	void Clear();
	//static void RegisterScriptApi(ApiInitializer &api);
protected:
	struct StringTableInfo {
		StringStringMap StringMap;
		XMLFile XMLTableTranslation;
		XMLFile XMLTable;
	};

	using StringTableMap = std::unordered_map < string, StringTableInfo > ;
	StringTableMap m_TableMap;

	bool Load(const string& TableName);
};

} //namespace DataClasses 
} //namespace MoonGlare 

#endif
