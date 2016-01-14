/*
  * Generated by cppsrc.sh
  * On 2015-02-27 11:39:53,85
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef Collision_H
#define Collision_H

namespace Physics {

/**
	Defines classess of collision.
	Alone it does not mean anything.
	It is used to create BodyClass and GroupMask.
*/
struct CollisionClass {
	enum e : unsigned __int16 {
//Objects types
		DynamicObject	= 0x0001,
		StaticObject	= 0x0002,
		MapObject		= 0x0004,
		//unused		= 0x0008,
		Object_ALL		= 0x000F,

//GhostObject types
		GhostObject		= 0x00F0,
		DamageZone		= 0x0010,
		EventZone		= 0x0020,
		//unused		= 0x0040,
		//unused		= 0x0080,

//??
		Player			= 0x0100,
//		Ally			= 0x0200,
//		Enemy			= 0x0400,
//		Collectable		= 0x0800, 
//		Type_All		= 0x0F00,

//??
		//unusedclass	= 0xF000,

		World			= 0,
		None			= 0,
		All				= 0xFFFF,//Object_ALL, // | Type_All,
	};
};


/**
	Defines body collision class.
*/
enum class BodyClass : unsigned __int16 {
	//collectable

	//eventzone
	//damagezone

	//player
	//enemy
	//ally

	//static
	//dynamic

	Default			= 0, //Object,

//deprecated
	Map				= CollisionClass::MapObject,
	StaticObject	= CollisionClass::StaticObject,
	Object			= CollisionClass::DynamicObject,
	Player			= CollisionClass::Player,		
};

/**
	Defines body group mask.
*/
enum class GroupMask : unsigned __int16 {
	Default			= CollisionClass::All,//CollisionClass::Object_ALL | CollisionClass::Type_All,
	None			= 0,

//deprecated
	Map				= CollisionClass::All,// CollisionClass::Type_All | CollisionClass::DynamicObject,
	StaticObject	= CollisionClass::All,// CollisionClass::Object_ALL | CollisionClass::DynamicObject,
	Object			= CollisionClass::All,// CollisionClass::Object_ALL | CollisionClass::Type_All,
	Player			= CollisionClass::All,// CollisionClass::Object_ALL | CollisionClass::Type_All,
};

struct CollisionMask {
	union {
		struct {
			BodyClass Body;
			GroupMask Group;

			GroupMask NotifyGroup;
			__int16 unused;
		};
		unsigned __int64 uValue;
	};
	void Set(GroupMask gm) { Group = gm; }
	void SetNotifyGroup(GroupMask gm) { NotifyGroup = gm; }
	void Set(BodyClass bc) { Body = bc; }

	CollisionMask(GroupMask gm = GroupMask::Default, BodyClass bc = BodyClass::Default, GroupMask notify = GroupMask::None) {
		Set(gm);
		Set(bc);
		SetNotifyGroup(notify);
	}
};

struct BodyClassEnumCoverter : GabiLib::EnumConverter < BodyClass, BodyClass::Default > {
	BodyClassEnumCoverter() {
		Add("Default", Enum::Default);
	}
};

struct GroupMaskEnumCoverter : GabiLib::EnumConverter < GroupMask, GroupMask::Default > {
	GroupMaskEnumCoverter() {
		Add("Default", Enum::Default);
		Add("None", Enum::None);

		//Add("StaticObject", Enum::StaticObject);
	}
};

struct BodyClassEnum : public GabiLib::EnumConverterHolder < BodyClassEnumCoverter > {
	static void LoadFromXML(const xml_node parent, CollisionMask& out);
};

struct GroupMaskEnum : public GabiLib::EnumConverterHolder < GroupMaskEnumCoverter > {
	static void LoadFromXML(const xml_node parent, CollisionMask& out);
};

} //namespace Physics 

#endif
