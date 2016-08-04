/*
 * Interfaces.h
 *
 *  Created on: 03-11-2013
 *      Author: Paweu
 */
#ifndef INTERFACESH
#define INTERFACES_H

namespace MoonGlare {
namespace Core {

class Interfaces {
public:
	Interfaces() = delete;
	static void DumpLists(ostream &out);
	static void RegisterScriptApi(ApiInitializer &api);
private:
};

} //namespace Core
} //namespace MoonGlare 

#endif // CINTERFACESLIST_H_ 
