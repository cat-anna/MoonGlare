/*
 * Config.h
 *
 *  Created on: 06-12-2013
 *      Author: Paweu
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#ifdef _BUILDING_TOOLS_

#define _DISABLE_FREEIMAGE_LIB_

#endif

//Common defines
#define VoidAction		do { /* NOP */ } while(false)

#ifdef DEBUG
#define DEBUG_TRUE true
#define RELEASE_TRUE false

#include "Debug.h"
namespace Config {
	namespace Current = Debug;
}

#else
#define DEBUG_TRUE false
#define RELEASE_TRUE true

#include "Release.h"
namespace Config {
	namespace Current = Release;
}
#endif

//This header takes care by itself
#include "DebugInterface.h"

namespace Config {
}

#endif // CONFIG_H_
