/*
  * Generated by cppsrc.sh
  * On 2016-09-24  9:36:12,25
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include PCH_HEADER
#include "CustomType.h"

namespace MoonGlare {
namespace TypeEditor {

//----------------------------------------------------------------------------------
using MapType = std::unordered_map<std::string, std::shared_ptr<const TypeEditorInfo>>;

MapType *gTypeInfoMap = nullptr;

void TypeEditorInfo::RegisterTypeEditor(std::shared_ptr<const TypeEditorInfo> typeinfo, const std::string &Name) {
	if (!gTypeInfoMap)
		gTypeInfoMap = new MapType();
	(*gTypeInfoMap)[Name].swap(typeinfo);
}

std::shared_ptr<const TypeEditorInfo> TypeEditorInfo::GetEditor(const std::string &Name) {
	if (!gTypeInfoMap)
		return nullptr;
	auto it = gTypeInfoMap->find(Name);
	if (it == gTypeInfoMap->end())
		return nullptr;
	return it->second;
}

} //namespace TypeEditor 
} //namespace MoonGlare 
