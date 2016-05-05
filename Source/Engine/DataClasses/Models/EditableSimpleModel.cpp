/*
 * cEditableSimpleModel.cpp
 *
 *  Created on: 03-01-2014
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>
#include "EditableSimpleModel.h"

namespace DataClasses {
namespace Models {

SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(EditableSimpleModel)

EditableSimpleModel::EditableSimpleModel(const string& Name) : BaseClass(Name) {
	SetName(Name);
}

EditableSimpleModel::~EditableSimpleModel() {
	 BaseClass::Finalize();
}

bool EditableSimpleModel::DoInitialize() {
	return BaseClass::DoInitialize();
}

bool EditableSimpleModel::DoFinalize() {
	return BaseClass::DoFinalize();
}

} // namespace Models
} // namespace DataClasses
