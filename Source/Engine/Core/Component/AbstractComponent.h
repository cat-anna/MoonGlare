/*
  * Generated by cppsrc.sh
  * On 2015-08-19 22:57:41,79
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef AbstractComponent_H
#define AbstractComponent_H

namespace MoonGlare {
namespace Core {
namespace Component {

class AbstractComponent
	: public Config::Current::DebugMemoryInterface {
public:
	AbstractComponent(ComponentManager *Owner);
	virtual ~AbstractComponent();

	virtual bool Initialize() = 0;
	virtual bool Finalize() = 0;

	virtual void Step(const MoveConfig &conf) = 0;

	virtual bool Load(xml_node node, Entity Owner, Handle &hout) = 0;
	virtual bool GetInstanceHandle(Entity Owner, Handle &hout) = 0;

	virtual bool Create(Entity Owner, Handle &hout);

	virtual bool LoadComponentConfiguration(pugi::xml_node node);

	/** 
		Push component entry to lua for manipulation by scripts. May not be reimplemented.
		@return false if not supported otherwise true (even if handle is not valid)
	*/
	virtual bool PushEntryToLua(Handle h, lua_State *lua, int &luarets);


//	virtual bool IsHandleValid(Handle h) = 0;
//	virtual bool Allocate(Entity owner, Handle &out) = 0;
//	virtual bool Release(Handle h) = 0;
//	virtual bool Release(Entity e) = 0;
//	virtual Handle GetHandle(Entity e) = 0;

	ComponentManager* GetManager() { return m_Owner; }

protected:
	HandleTable* GetHandleTable() { return m_HandleTable; }
private:
	ComponentManager *m_Owner;
	HandleTable *m_HandleTable;
	void *m_padding1;
	void *m_padding2;
};

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 

#endif
