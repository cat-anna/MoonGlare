/*
* Debug.cpp
*
* Source file for debug configuration
*
*  Created on: 03-08-2013
*      Author: Paweu
*/
#include <pch.h>
#ifdef RELEASE
#include <nfMoonGlare.h>

namespace Config {
namespace Release {

//---------------------------------------------------------------------------------------

volatile bool _ThreadCanContinue;

void DebugThread() {
#if 0
	while (_ThreadCanContinue) {
	}
#endif // 0
}

void Initialize() {
	//_ThreadCanContinue = true;
	//std::thread(&DebugThread).detach();
}
void Finalize() {
	//_ThreadCanContinue = false;
}

//---------------------------------------------------------------------------------------

} //namespace Release
} //namespace Config

#endif
