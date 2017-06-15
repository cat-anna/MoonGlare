/*
 * ApiOutCalls.h
 *
 *  Created on: 28-10-2013
 *      Author: Paweu
 */

#ifndef APIOUTCALLS_H_
#define APIOUTCALLS_H_

#include "ApiBaseType.h"
#include "ApiDefinition.h"


class ApiOutCalls {
public:
	struct sFunDef {
		std::string Caller;
		ApiFunction *FunDef;
		sFunDef():Caller(), FunDef(0){ };
		~sFunDef() { delete FunDef; }
	};
	typedef std::vector<sFunDef*> FunList;
private:
	FunList m_FunList;
	ApiDefinition *m_ApiDef;
public:
	ApiOutCalls(ApiDefinition *ApiDef);
	~ApiOutCalls();
	bool LoadFromXML(const pugi::xml_node Node);
	bool Load(const std::string &Name);

	const ApiFunction* GetFunction(const std::string &FunName, const std::string &Caller = "") const;
	void Clear();
};

#endif /* APIOUTCALLS_H_ */
