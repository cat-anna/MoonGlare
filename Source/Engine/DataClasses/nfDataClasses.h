/*
 * DataClasses.h
 *
 *  Created on: 20-10-2013
 *      Author: Paweu
 */

#ifndef NFDATACLASSES_H_
#define NFDATACLASSES_H_

enum class DataPath {
	Root,   

	Maps,
	Models,
	Fonts,
	Scenes,
	Shaders,
	Scripts,
	Sounds,
	Music,
	Texture,
	XML,
	Tables,
	Objects,

	MaxValue,
};

#define xmlAttr_Shared				"Shared"
#define xmlAttr_Class				"Class"
#define xmlAttr_Name				"Name"
#define xmlAttr_Object				"Object"
#define xmlAttr_SubXML				"File"
#define xmlAttr_File				"File"
#define xmlAttr_Type				"Type"
#define xmlAttr_Type				"Type"
#define xmlAttr_Value				"Value"
#define xmlAttr_AutoDetect			"AutoDetect"
#define xmlAttr_Id					"Id"

#define xmlNode_ScriptEvents		"Events"

#define InternalStringTable			"MoonGlare"
#define MenusStringTable			"Menus"

namespace DataClasses {

	template<class T>
	struct ResourceFinalizer {
		void operator()(T *t) {
			if (!t->Finalize()) {
				AddLog(Error, "Unable to finalize resource " << t->GetName());
			}
			delete t;
		}
	};

	namespace Fonts {
		class Wrapper;
		using FontInstance = std::unique_ptr < Wrapper > ;
		class iFont;
		using FontPtr = std::shared_ptr < iFont > ;

		enum class FontStyleFlags {
			Bold		= 0x01,
			Italic		= 0x02,
			Underline	= 0x04,
			StrikeOut	= 0x08,
		};

		struct Descriptor {
			float Size = 0.0f;
			Graphic::vec3 Color = Graphic::vec3(1.0f);

			//DefineFlag(m_Flags, (unsigned)FontStyleFlags::Bold, Bold);
			//DefineFlag(m_Flags, (unsigned)FontStyleFlags::Italic, Italic);
			//DefineFlag(m_Flags, (unsigned)FontStyleFlags::Underline, Underline);
			//DefineFlag(m_Flags, (unsigned)FontStyleFlags::StrikeOut, StrikeOut);
		protected:
			//unsigned m_Flags = 0;
		};
	}
	using Fonts::FontPtr;
	using Fonts::FontInstance;

	namespace Maps {
		class iMap;
		struct iMapDeleter {
			void operator()(iMap*);
		};
		using MapPtr = std::shared_ptr < iMap >;
		using MapClassRegister = GabiLib::DynamicClassRegisterDeleter < iMap, iMapDeleter, const string& > ;
	}
	using Maps::iMap;
	using Maps::MapPtr;

	namespace Paths {
		class iPath;
		class PathRegister;

		using iPathSharedPtr = std::shared_ptr < iPath > ;
		using iPathWeakPtr = std::weak_ptr < iPath > ;
	}

	class DataClass;
	class StringTable;

//	using FileList = std::list < std::pair<string, DataModule*> > ;

	struct ModuleSettings {
		string ConsoleFont;
		string FirstScene;

		void Append(const ModuleSettings &cfg) {
			if (!cfg.ConsoleFont.empty()) ConsoleFont = cfg.ConsoleFont;
			if (!cfg.FirstScene.empty()) FirstScene = cfg.FirstScene;
		}

		//LoadMeta | SaveMeta
		bool LoadMeta(const xml_node node) {
			if (!node) return true;
			XML::ReadTextIfPresent(node, "ConsoleFont", ConsoleFont);
			XML::ReadTextIfPresent(node, "FirstScene", FirstScene);
			return true;
		}
	};
}

using DataClasses::Maps::MapPtr;
using DataClasses::Paths::iPathSharedPtr;

#include "Models\nfModels.h"

#endif // DATACLASSES_H_ 
