/*
 * MoveControlers.h
 *
 *  Created on: 20-10-2013
 *      Author: Paweu
 */

#ifndef MOVECONTROLERS_H_
#define MOVECONTROLERS_H_

#include "iMoveController.h"

namespace MoveControllers {

struct KeyFlags {
	enum {
		Move_Up			= 0x00000001,
		Move_Down		= 0x00000002,
		Move_Left		= 0x00000004,
		Move_Right		= 0x00000008,
		Move_Mask		= 0x0000000F,

		Turn_Up			= 0x00000010,
		Turn_Down		= 0x00000020,
		Turn_Left		= 0x00000040,
		Turn_Right		= 0x00000080,
		Turn_Mask		= 0x000000F0,

		Run				= 0x00000100,
		Crouch			= 0x00000200,
		Jump			= 0x00000400,
		//??				= 0x00000800,
		Action_Mask		= 0x00000F00,
		
		UnusedSection_0	= 0x0000F000,

		User_A			= 0x00010000,
		User_B			= 0x00020000,
		User_C			= 0x00040000,
		User_D			= 0x00080000,
		User_E			= 0x00100000,
		User_F			= 0x00200000,
		User_G			= 0x00400000,
		User_H			= 0x00800000,
		User_Mask		= 0x00FF0000,

		UnusedSection_1	= 0x0F000000,
		UnusedSection_2	= 0xF0000000,
	};
};

}// namespace MoveControllers

#endif // MOVECONTROLERS_H_ 
