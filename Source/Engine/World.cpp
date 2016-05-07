/*
  * Generated by cppsrc.sh
  * On 2016-05-03 21:31:13,10
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "World.h"

namespace MoonGlare {

World::World() {
}

World::~World() {
}

//------------------------------------------------------------------------------------------

bool World::Initialize() {
	if (!m_EntityManager.Initialize()) {
		AddLogf(Error, "Failed to initialize EntityManager!");
		return false;
	}

	return true;
}

bool World::Finalize() {
	if (!m_EntityManager.Finalize()) {
		AddLogf(Error, "Failed to finalize EntityManager!");
	}

	return true;
}

} //namespace MoonGlare 