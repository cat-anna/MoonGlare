/*
	Generated by cppsrc.sh
	On 2014-12-25 23:39:59,76
	by Paweu
*/

#include <pch.h>
#include <MoonGlare.h>
#include "InputMap.h"

namespace Core {

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(Input);
MouseEventDispatcher *Input::_MouseDispatcher = nullptr;

Input::Input(): m_MouseDelta(0, 0) {
	SetThisAsInstance();
}

Input::~Input() {
}

//-------------------------------------------------------------------------------------------------

bool Input::Initialize() {
	return true;
}

bool Input::Finalize() {
	Clear();
	return true;
}

//-------------------------------------------------------------------------------------------------

void MouseEventDispatcher::Set() {
	Input::SetMouseDispatcher(this);
}

void MouseEventDispatcher::Revoke() {
	Input::RevokeMouseDispatcher(this);
}

} //namespace Core 
