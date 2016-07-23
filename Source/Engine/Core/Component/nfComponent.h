#pragma once
#ifndef nfComponent_H
#define nfComponent_H

namespace MoonGlare {
namespace Core {
namespace Component {

class AbstractComponent;
using UniqueAbstractComponent = std::unique_ptr<AbstractComponent>;

class ComponentManager;

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 

#endif
