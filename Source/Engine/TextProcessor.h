/*
  * Generated by cppsrc.sh
  * On 2016-10-12 18:39:40,06
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef TextProcessor_H
#define TextProcessor_H

namespace MoonGlare {

class TextProcessor {
public:
 	TextProcessor(DataClasses::StringTable *Tables = nullptr);
 	~TextProcessor();

	void SetTables(DataClasses::StringTable *Tables) { m_StringTables = Tables; }
	void Process(const std::string& input, std::string &out);
protected:
	DataClasses::StringTable *m_StringTables;
};

} //namespace MoonGlare 

#endif