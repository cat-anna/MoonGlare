/*
  * Generated by cppsrc.sh
  * On 2015-08-19 22:57:41,79
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <nfMoonGlare.h>
#include "ComponentManager.h"
#include "AbstractComponent.h"

namespace MoonGlare {
namespace Core {
namespace Component {

AbstractComponent::AbstractComponent(ComponentManager * Owner)
		: m_Owner(Owner) {
	m_HandleTable = Owner->GetWorld()->GetHandleTable();
}

AbstractComponent::~AbstractComponent() {
}

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 
