/*
 * cModel.cpp
 *
 *  Created on: 12-11-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>

namespace MoonGlare {
namespace DataClasses {
namespace Models {

SPACERTTI_IMPLEMENT_ABSTRACT_CLASS(iModel)

iModel::iModel(const string& Name) :
		BaseClass(Name) {
}

iModel::~iModel() {
	Finalize();
}

//------------------------------------------------------------------------------------------------

Physics::SharedShape iModel::ConstructShape(float ShapeScale) const {
	return Physics::SharedShape();
}

const Physics::PhysicalProperties* iModel::GetPhysicalProperties() const {
	return &m_PhysicalProperties;
}

//------------------------------------------------------------------------------------------------

bool iModel::DoInitialize() {
	return BaseClass::DoInitialize();
}

bool iModel::DoFinalize() {
	return BaseClass::DoFinalize();
}

} // namespace Models 
} // namespace DataClasses 
} //namespace MoonGlare 
