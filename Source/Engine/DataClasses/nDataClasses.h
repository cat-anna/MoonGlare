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
using ModelResPtr = MoonGlare::DataClasses::SmartResource<DataClasses::ModelPtr>;

//#include "Paths/iPath.h"
//#include "Paths/PathRegister.h"

#include "Models/nModels.h"

#include "Texture.h"
#include "StringTable.h"

//module
#include "DataModule.h"

#endif // DATACLASSES_H_
