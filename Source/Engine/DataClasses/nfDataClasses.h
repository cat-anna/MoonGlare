/*
 * DataClasses.h
 *
 *  Created on: 20-10-2013
 *      Author: Paweu
 */

#ifndef NFDATACLASSES_H_
#define NFDATACLASSES_H_

#define xmlAttr_Shared				"Shared"
#define xmlAttr_Class				"Class"
#define xmlAttr_Name				"Name"
#define xmlAttr_Object				"Object"
#define xmlAttr_SubXML				"File"
#define xmlAttr_File				"File"
#define xmlAttr_Type				"Type"
#define xmlAttr_Value				"Value"
#define xmlAttr_Id					"Id"

#define InternalStringTable			"MoonGlare"
#define MenusStringTable			"Menus"

namespace MoonGlare {
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

	class DataClass;
	class StringTable;
}
} //namespace MoonGlare 

//using MoonGlare::DataClasses::Paths::iPathSharedPtr;

#include "Models\nfModels.h"

#endif // DATACLASSES_H_ 
