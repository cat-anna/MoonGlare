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
} //namespace Scripts
} //namespace Core

using ::Core::Scripts::iScriptEvents;

namespace MoonGlare {
namespace Core {
namespace Scripts {

	class eLuaPanic : public std::runtime_error {
	public:
		eLuaPanic(const string &msg) : std::runtime_error(msg) {}
	};

	class cScriptEngine;

} //namespace Scripts
} //namespace Core
} //namespace MoonGlare

#include "LuaApi.h"

#endif // NSCRIPTS_H_
