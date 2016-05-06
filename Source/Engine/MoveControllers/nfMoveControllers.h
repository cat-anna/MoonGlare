/*
 * MoveControlers.h
 *
 *  Created on: 20-10-2013
 *      Author: Paweu
 */

#ifndef NFMOVECONTROLERS_H_
#define NFMOVECONTROLERS_H_

namespace MoveControllers {
	class iMoveController;

	using iMoveControllerPtr = std::unique_ptr<iMoveController>;

	using MoveControllerClassRegister = Space::DynamicClassRegister < iMoveController > ;
}

//Reveal interfaces
using MoveControllers::iMoveControllerPtr;

#endif // MOVECONTROLERS_H_
