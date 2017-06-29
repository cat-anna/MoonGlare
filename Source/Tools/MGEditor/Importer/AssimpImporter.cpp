/*
  * Generated by cppsrc.sh
  * On 2017-01-29 21:22:14,23
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#include PCH_HEADER
#include "AssimpImporter.h"

#include <icons.h>
#include <Module.h>
#include <iEditor.h>
#include <iFileIconProvider.h>

namespace MoonGlare::Editor::Importer {

class AssimpImporterInfo
	: public QtShared::iModule	
	, public QtShared::iEditorInfo {
public:
	AssimpImporterInfo(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {}

	virtual std::vector<FileHandleMethodInfo> GetOpenFileMethods() const override {
		return std::vector<FileHandleMethodInfo> {
//			FileHandleMethodInfo{ "epx", ICON_16_ENTITYPATTERN_RESOURCE, "Edit entity pattern", "open", },
		};
	}
};

QtShared::ModuleClassRgister::Register<AssimpImporterInfo> AssimpImporterInfoReg("AssimpImporter");

//----------------------------------------------------------------------------------

AssimpImporter::AssimpImporter() {

}

AssimpImporter::~AssimpImporter() {

}

} //namespace MoonGlare::Editor::Importer 

