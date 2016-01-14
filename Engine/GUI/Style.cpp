/*
  * Generated by cppsrc.sh
  * On 2015-03-07 20:53:21,59
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include "GUI.h"
#include <MoonGlare.h>

namespace MoonGlare {
namespace GUI {

GABI_IMPLEMENT_STATIC_CLASS(Style);

Style::Style(StyleSet *Owner, const string& Name):
		BaseClass(),
		m_Owner(Owner),
		m_Name(Name) {

}

Style::~Style() {
}

//----------------------------------------------------------------

void Style::Clone(const Style& s) {
	Font = s.Font;
	FontStyle = s.FontStyle;
	BaseColor = s.BaseColor;
}

bool Style::LoadMeta(xml_node node) {
	if (!node) return false;

	auto *fontname = node.child("Font").text().as_string(ERROR_STR);
	Font = GetDataMgr()->GetFont(fontname);
	//if (!Font) 
		//return false; 

	if (!XML::Vector::Read(node, "BaseColor", BaseColor, math::vec3(1), XML::Captions::RGBA)) {
		AddLog(Warning, "Uanble to read base color for style! Ignored.");
	}

	if (!XML::Vector::Read(node, "FontColor", this->FontStyle.Color, BaseColor, XML::Captions::RGBA)) {
		AddLog(Warning, "Unable to read color for font! Ignored.");
	}

	FontStyle.Size = node.child("Size").attribute(xmlAttr_Value).as_float(0);
	//auto Flags = node.child("Flags");
	//if (Flags) {
		//FontStyle.SetBold(Flags.attribute("Bold").as_bool(false));
		//FontStyle.SetItalic(Flags.attribute("Italic").as_bool(false));
		//FontStyle.SetUnderline(Flags.attribute("Underline").as_bool(false));
		//FontStyle.SetStrikeOut(Flags.attribute("StrikeOut").as_bool(false));
	//}

	return true;
}

//----------------------------------------------------------------

SharedAnimation Style::GetAnimation(const string& Name) const { 
	return m_Owner->GetAnimation(this, Name); 
}

} //namespace GUI 
} //namespace MoonGlare 

