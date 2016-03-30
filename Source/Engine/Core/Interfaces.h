/*
 * Interfaces.h
 *
 *  Created on: 03-11-2013
 *      Author: Paweu
 */
#ifndef INTERFACESH
#define INTERFACES_H
namespace Core {

class Interfaces {
public:
	static void DumpLists(ostream &out);

	static void Initialize();

	static void RegisterScriptApi(ApiInitializer &api);
private:
	Interfaces();

	static Interfaces *Instance;
};

} //namespace Core
#endif // CINTERFACESLIST_H_ 
