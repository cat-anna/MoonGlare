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

using MapResPtr = DataClasses::SmartResource<DataClasses::MapPtr>;
using FontResPtr = DataClasses::SmartResource<DataClasses::FontPtr>;
using ModelResPtr = DataClasses::SmartResource<DataClasses::ModelPtr>;

#include "Paths/iPath.h"
#include "Paths/PathRegister.h"

#include "Models/nModels.h"
#include "iMap.h"

#include "Texture.h"
#include "StringTable.h"

//module
#include "DataModule.h"

#endif // DATACLASSES_H_
