 #ifndef DataModuleH
#define DataModuleH
namespace DataClasses {

#define xmlstr_Module_xml			"Module.xml"

//XML node names definition

#define xmlstr_Scenes				"Scenes"
#define xmlstr_ModuleName			"ModuleName"
#define xmlstr_FirstScene			"FirstScene"
#define xmlstr_ConsoleFont			"ConsoleFont"
#define xmlstr_DataModule			"DataModule"
#define xmlstr_Paths				"Paths"
#define xmlstr_Models				"Models"
#define xmlstr_Fonts				"Fonts"
#define xmlstr_PatternObjects		"PatternObjects"

#define xmlstr_Class				"Class"
#define xmlstr_Name					"Name"

#define xmlRes_SceneMap				"Map"

#define xmldef_SceneDef				"Scene"
#define xmldef_LoadingSceneDef		"Loading"

#define xmlModuleNode_Maps			"Maps"
#define xmlModuleNode_Models		"Models"
#define xmlModuleNode_Fonts			"Fonts"
#define xmlModuleNode_Scenes		"Scenes"
#define xmlModuleNode_Shaders		"Shaders"
#define xmlModuleNode_Scripts		"Scripts"
#define xmlModuleNode_Sounds		"Sounds"
#define xmlModuleNode_Music			"Music"
	
#define xmlModuleNode_Player		"Player"
#define xmlModuleNode_Options		"Options"
#define xmlModuleNode_PredefObjects	"PredefObjects"
#define xmlModuleNode_GameEngine	"GameEngine"

DECLARE_SCRIPT_EVENT_VECTOR(ModuleScriptEvents, ::Core::Scripts::iScriptEvents,
		SCRIPT_EVENT_ADD(),
		SCRIPT_EVENT_REMOVE());

class DataModule : public cRootClass {
	GABI_DECLARE_CLASS_NOCREATOR(DataModule, cRootClass);
	DECLARE_SCRIPT_HANDLERS_ROOT(ModuleScriptEvents);
public:
	DataModule(int/*FileSystem::iContainer *container*/);
	~DataModule();

	bool IsValid() const;
	virtual bool Open();

	const xml_node GetConfig() const { return GetModuleDocNode().child(xmlModuleNode_Options); }

	DefineREADAcces(ModuleName, string);
	//DefineDirectGetterConst(Container, FileSystem::iContainer*);

	MapPtr LoadResource(const MapResPtr *res, ::Core::GameScene *Owner);

	::Core::Scene::ciScene* LoadScene(const string& Name, const string& Class);
//old
	bool LoadGameEgnine();

	//accessors
	DefineFlagGetter(m_Flags, MF_Opened, Opened)

	enum {
		MF_Opened = 0x00001000,
	};

	static void RegisterScriptApi(::ApiInitializer &api);
protected:
	//FileSystem::iContainer *m_Container;

	unsigned m_Flags, m_OwnedResources;
	string m_ModuleName;
	FileSystem::XMLFile m_ModuleDoc, m_PredefObjectsDoc;

	DataModule();

	const xml_node GetModuleDocNode();
	const xml_node GetModuleDocNode(const char* Name = 0) const;

	bool LoadPrivateXML(FileSystem::XMLFile &doc, const string& file, DataPath origin) const;

//old
	DefineFlagSetter(m_Flags, MF_Opened, Opened)
};

GenExceptionMsg(eContainerError, "Data container fatal error!");
GenExceptionMsg(eLoadError, "Loading data module failed");

} //namespace DataClasses

#endif // DataModule_H
