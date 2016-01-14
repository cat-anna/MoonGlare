#pragma once

namespace Graphic {

struct MatrixStack {
	cRenderDevice &Device;

	MatrixStack(cRenderDevice &dev):
			Device(dev),
			m_Stack(),
			m_Top(0) {
		Reset();
	}

	void Translate(const vec3& delta) {
		*m_Top = glm::translate(*m_Top, delta);
	}
	void Translate(const vec2& delta) {
		*m_Top = glm::translate(*m_Top, vec3(delta, 0));
	}

	void Scale(const vec3& scale) {
		*m_Top = glm::scale(*m_Top, scale);
	}
	void Scale(const vec2& scale) {
		*m_Top = glm::scale(*m_Top, vec3(scale, 1.0f));
	}
	void Scale(float value) {
		*m_Top = glm::scale(*m_Top, vec3(value));
	}

	void Push() {
		assert(m_Top <= &m_Stack.back());
		*(m_Top + 1) = *m_Top;
		++m_Top;
	}

	void Pop() {
		assert(m_Top >= &m_Stack.front());
		--m_Top;
	}

	void Reset() {
		m_Top = &m_Stack[0];
		*m_Top = mat4();
	}

	void Update() const {
		Device.SetModelMatrix(*m_Top);
	}

	MatrixStack(const MatrixStack&) = delete;
	MatrixStack& operator=(const MatrixStack&) = delete;
private:
	mat4 *m_Top;
	std::array<mat4, 16> m_Stack;
};

}
