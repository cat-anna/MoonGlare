/*
 * Constants.h
 *
 *  Created on: 09-01-2014
 *      Author: Paweu
 */
#ifndef CONSTANTS_H_
#define CONSTANTS_H_
namespace Graphic {
namespace Flags {

enum {
	fBGRA				= GL_BGRA,
	fBGR				= GL_BGR,
	fRGBA				= GL_RGBA,
	fRGB				= GL_RGB,
	LuminanceAlpha		= GL_LUMINANCE_ALPHA,


	fUnsignedChar	= GL_UNSIGNED_BYTE,
	fUnsignedInt	= GL_UNSIGNED_INT,

	fQuads		 = GL_QUADS,
	fTriangles	 = GL_TRIANGLES,
};

} //namespace Flags
} //namespace Graphic
#endif /* CONSTANTS_H_ */
