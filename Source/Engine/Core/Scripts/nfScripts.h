/*
 * nScripts.h
 *
 *  Created on: 15-12-2013
 *      Author: Paweu
 */

#ifndef NSCRIPTS_H_
#define NSCRIPTS_H_

namespace Core {
	namespace Scripts {
		class iScriptEvents;

		class Script;
		using SharedScript = std::shared_ptr < Script > ;

		class cScriptEngine;
	} //namespace Scripts
	using Scripts::cScriptEngine;
} //namespace Core

using ::Core::Scripts::iScriptEvents;

#include "LuaApi.h"

#endif // NSCRIPTS_H_
