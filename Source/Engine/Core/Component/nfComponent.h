#pragma once

#include <Foundation/Component/Entity.h>

#define DEFINE_COMPONENT_PROPERTY(NAME)						\
decltype(m_##NAME) Get ## NAME() const { return m_##NAME; }	\
void Set ## NAME(const decltype(m_##NAME) &v) { m_##NAME = v; SetDirty(); }	

#define DEFINE_COMPONENT_PROPERTY_CAST(NAME, TYPE)					\
TYPE Get ## NAME() const { return static_cast<TYPE>(m_##NAME); }	\
void Set ## NAME(const TYPE &v) { m_##NAME = static_cast<decltype(m_##NAME)>(v); SetDirty(); }	

#define DEFINE_COMPONENT_FLAG(NAME, LOCATION)				\
bool Get ## NAME() const { return LOCATION; }				\
void Set ## NAME(bool v) {LOCATION = v; SetDirty(); }	

namespace MoonGlare {
namespace Core {
namespace Component {

using namespace MoonGlare::Component;

class SubsystemManager;

} //namespace Component 

using Component::SubSystemId;

} //namespace Core 
} //namespace MoonGlare 
