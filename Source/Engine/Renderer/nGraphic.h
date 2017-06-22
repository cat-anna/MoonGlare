/*
 * Graphic.h
 *
 *  Created on: 09-11-2013
 *      Author: Paweu
 */

#ifndef NGRAPHIC_H_
#define NGRAPHIC_H_

#include "Constants.h"
#include "glTypes.h"

#include "VAO.h"

#include "LoadQueue.h"

#include <Renderer/VirtualCamera.h>

#include "RenderDevice.h"

using Graphic::cRenderDevice;

namespace Graphic {
	inline cRenderDevice* GetRenderDevice() { return cRenderDevice::Instance(); }
}

#endif /* GRAPHIC_H_ */
