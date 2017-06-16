/*
 * DataClasses.h
 *
 *  Created on: 20-10-2013
 *      Author: Paweu
 */

#ifndef DATACLASSES_H_
#define DATACLASSES_H_

//interfaces
#include "DataClass.h"
#include "Resource.h"

using FontResPtr = MoonGlare::DataClasses::SmartResource<DataClasses::FontPtr>;
using ModelResPtr = DataClasses::ModelPtr;

#include "Models/iModel.h"

#include "StringTable.h"

#endif // DATACLASSES_H_
