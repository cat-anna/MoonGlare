/*
  * Generated by cppsrc.sh
  * On 2015-08-02 13:34:51,74
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "MoonGlareInisder.h"
#include "ResourceEnumerator.h"

#include <Foundation/LuaUtils.h>

namespace MoonGlare {
namespace Debug {
namespace Insider {

SPACERTTI_IMPLEMENT_STATIC_CLASS(ResourceEnumerator);
RegisterApiDerivedClass(ResourceEnumerator, &ResourceEnumerator::RegisterScriptApi);

ResourceEnumerator::ResourceEnumerator(InsiderMessageBuffer &buffer, MessageTypes Type):
		BaseClass(),
		m_Buffer(buffer),
		m_Type(Type),
		m_ElementCount(0),
		m_ListBase(nullptr){
	buffer.Clear();
	buffer.GetHeader()->MessageType = Type;
	m_ListBase = buffer.AllocAndZero<PayLoad_ListBase>();
	m_ListBase->Count = 0;
}

ResourceEnumerator::~ResourceEnumerator() {
}

//----------------------------------------------------------------

void ResourceEnumerator::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("InsiderResourceEnumerator")
		.addCFunction("LuaElement", &ThisClass::LuaElement)
	.endClass();
}

//----------------------------------------------------------------

void ResourceEnumerator::finish() {
	m_ListBase->Count = m_ElementCount;
}

//----------------------------------------------------------------

int ResourceEnumerator::LuaElement(lua_State *lua) {
	auto *item = m_Buffer.AllocAndZero<PayLoad_LuaElement_Item>();

	auto name = Utils::Scripts::Lua_to<const char *>(lua, -2);
	auto type = lua_type(lua, -1);
	auto value = Utils::Scripts::Lua_to<const char *>(lua, -1);

	item->LuaType = (u8)type;
	item->NameLen = (u16)((name ? strlen(name) : 0) + 1);
	item->ValueLen = (u16)((value ? strlen(value) : 0) + 1);
	item->Index = (u16)m_ElementCount++;
	m_Buffer.PushString(name);
	m_Buffer.PushString(value);

	return 0;
}

} //namespace Insider 
} //namespace Debug 
} //namespace MoonGlare 

