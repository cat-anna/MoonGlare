/*
  * Generated by cppsrc.sh
  * On 2015-05-19 17:27:02,94
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef ModulesManager_H
#define ModulesManager_H

namespace MoonGlare {
namespace Modules {

#define DEFINE_MODULE(CLASS) static CLASS CLASS##Module

/** Modules are sorted by this enum */
enum class ModuleType {
	Unknown, Core, Resources, Functional, Application, Debug,
};
struct ModuleTypeEnumConverter : GabiLib::EnumConverter < ModuleType, ModuleType::Unknown > {
	ModuleTypeEnumConverter() {
		Add("Unknown", Enum::Unknown);
		Add("Default", Enum::Unknown);
		Add("Application", Enum::Application);
		Add("Core", Enum::Core);
		Add("Resources", Enum::Resources);
		Add("Functional", Enum::Functional);
		Add("Debug", Enum::Debug);
	}
};
using ModuleTypeEnum = GabiLib::EnumConverterHolder < ModuleTypeEnumConverter > ;

enum class NotifyEvent {
	None,
};

struct ModuleDescription {
	const wchar_t *Author;
	const wchar_t *TextInfo;
};

struct ModuleInfo {
public:
	ModuleInfo(const ModuleInfo&) = delete;
	ModuleInfo& operator=(const ModuleInfo&) = delete;
	ModuleInfo(const char *Name, ModuleType Type);
	virtual ~ModuleInfo();

	virtual bool Initialize();
	virtual bool Finalize();

	virtual void Notify(NotifyEvent event);
	virtual void Notify(SettingsGroup what);

	virtual const ModuleDescription* GetDescription() const;

	DefineDirectGetter(Name, const char *);
	DefineDirectGetterConst(Type, ModuleType);
protected:
	using ModuleType = ModuleType;
	using BaseClass = ModuleInfo;
private:
	const char *m_Name;
	const ModuleType m_Type;
};

class ModulesManager : public cRootClass {
	GABI_DECLARE_CLASS_SINGLETON(ModulesManager, cRootClass);
public:
 	ModulesManager();
 	virtual ~ModulesManager();

	bool Initialize();
	bool Finalize();

	void BroadcastNotification(NotifyEvent event);
	void BroadcastNotification(SettingsGroup what);

	void DumpModuleList(std::ostream &out);

	enum class Flags {
		Initialized,
	};
	DefineFlagGetter(m_Flags, FlagBit(Flags::Initialized), Initialized);
private: 
	unsigned m_Flags;

	DefineFlagSetter(m_Flags, FlagBit(Flags::Initialized), Initialized);
};

} //namespace Modules 

inline Modules::ModulesManager* GetModulesManager() { return Modules::ModulesManager::Instance(); }

} //namespace MoonGlare 

#endif
