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

#include "Window.h"

#include "Texture.h"
#include "VAO.h"
#include "SkyCube.h"
#include "FrameBuffer.h"

#include "Material.h"
#include "StaticFog.h"
#include "Environment.h"

#include "LoadQueue.h"

#include <Renderer/VirtualCamera.h>

#include "RenderDevice.h"

#include "Dereferred/nDereferred.h"

using Graphic::cRenderDevice;

namespace Graphic {
	inline cRenderDevice* GetRenderDevice() { return cRenderDevice::Instance(); }

	template<GLenum v>
	struct AutoGLEnabler {
		AutoGLEnabler() { glEnable(v); }
		~AutoGLEnabler() { glDisable(v); }
	};

	using StencilTestEnabler = AutoGLEnabler < GL_STENCIL_TEST > ;
	using DepthTestEnabler = AutoGLEnabler < GL_DEPTH_TEST > ;
}

#endif /* GRAPHIC_H_ */
