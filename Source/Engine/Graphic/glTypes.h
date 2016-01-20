#pragma once

namespace Graphic {
namespace Types {

struct TypeInfo {
	unsigned ElementSize;
	unsigned TypeSize;
	unsigned GLTypeValue;
	TypeInfo(unsigned aElementSize, unsigned aTypeSize, unsigned aGLTypeValue) {
		ElementSize = aElementSize;
		GLTypeValue = aGLTypeValue;
		TypeSize = aTypeSize;
	}

	TypeInfo() {
		ElementSize = 0;
		GLTypeValue = 0;
		TypeSize = 0;
	}
};

template <class T> 
struct glType {
	typedef T type;
//	static const unsigned type_value = value;
};

#define GENTYPE(TYPE, VALUE)	\
template <> \
struct glType<TYPE> {\
	typedef TYPE type;\
	static unsigned value() { return VALUE; }\
	static size_t type_size() { return sizeof(TYPE); } \
};	

#define FOR_GLEW(x) x

GENTYPE(float,			GL_FLOAT)
GENTYPE(         int,	GL_INT)
GENTYPE(unsigned int,	GL_UNSIGNED_INT)
GENTYPE(unsigned short,	GL_UNSIGNED_SHORT)
GENTYPE(unsigned char,	GL_UNSIGNED_BYTE)

} //namespace Types
} //namespace Graphic
