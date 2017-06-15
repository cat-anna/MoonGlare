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

SPACERTTI_IMPLEMENT_CLASS(iModel, DataClass);

iModel::iModel(const string& Name) :
    DataClass(Name),
    m_Meshes(),
    m_VAO() {
}

iModel::~iModel() {
    Finalize();
}

//------------------------------------------------------------------------------------------------

bool iModel::DoFinalize() {
    m_VAO.Finalize();
    m_Meshes.clear();
    return DataClass::DoFinalize();
}

} // namespace Models 
} // namespace DataClasses 
} // namespace MoonGlare 
