#ifndef GRAPHICMATERIAL_H
#define GRAPHICMATERIAL_H

namespace Graphic {

struct Material {
	Texture Texture;
	math::vec3 BackColor = math::vec3(1, 1, 1);
	float AlphaThreshold = 0.2f;

	Material() : Texture() { }
};

} // namespace Graphic
#endif // GRAPHICMATERIAL_H
