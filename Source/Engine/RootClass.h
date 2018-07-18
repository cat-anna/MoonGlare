#pragma once

//#include <Core/Scripts/LuaApi.h>

class cRootClass : public Space::RTTI::RTTIObject {
	SPACERTTI_DECLARE_CLASS(cRootClass, Space::RTTI::RTTIObject);
	//DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public: 
	static void RegisterScriptApi(ApiInitializer &api);
};
