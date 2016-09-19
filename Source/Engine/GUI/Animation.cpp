/*
  * Generated by cppsrc.sh
  * On 2015-03-06  9:44:21,50
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "GUI.h"

namespace MoonGlare {
namespace GUI {

SPACERTTI_IMPLEMENT_STATIC_CLASS(Animation);

Animation::Animation():
		BaseClass(),
		m_Speed(0.1f),
		m_StartFrame(0),
		m_EndFrame(0),
		m_FrameSpacing(0),
		m_FrameCount(1),
		m_DrawEnabled(false) {
}

Animation::~Animation() {
}

//----------------------------------------------------------------'

bool Animation::LoadMeta(const xml_node node) {
	if (!node) return false;

	auto cstrName = node.attribute(xmlAttr_Name).as_string(0);
	if (!cstrName) {
		AddLog(Error, "Unnamed animation definition. Ignored.");
		return false;
	}
	m_Name = cstrName;
	m_DrawEnabled = false;

	m_Speed = node.child("Speed").attribute(xmlAttr_Value).as_float(m_Speed);
	auto tex = node.child("Texture").text().as_string(ERROR_STR);

	auto nodeFrames = node.child("Frames");
	unsigned StartFrame = nodeFrames.attribute("Start").as_uint(0);
	unsigned FrameCount = nodeFrames.attribute("Count").as_uint(1);

	math::uvec2 FrameStripCount, FrameSpacing;
	auto nodeStrip = node.child("Strip");
	XML::Vector::Read(nodeStrip, "Size", m_FrameSize);
	XML::Vector::Read(nodeStrip, "Spacing", FrameSpacing);
	XML::Vector::Read(nodeStrip, "Count", FrameStripCount, math::uvec2(1, 1));

	return Load(tex, StartFrame, FrameCount, FrameStripCount, FrameSpacing, m_FrameSize, false);
}

bool Animation::Load(const std::string &fileuri, unsigned StartFrame, unsigned FrameCount, math::uvec2 FrameStripCount, math::uvec2 Spacing, math::vec2 FrameSize, bool Uniform) {
	m_StartFrame = StartFrame;
	m_EndFrame = m_StartFrame + FrameCount - 1;

	m_FrameCount = FrameStripCount;
	m_FrameSpacing = Spacing;

	if (FrameCount == 0) {
		FrameCount = 1;
		m_StartFrame = m_EndFrame = 0;
	}

	if (!GetFileSystem()->OpenTexture(m_Texture, fileuri, DataPath::Texture, true)) {
		AddLog(Error, "Unable to load texture file for animation!");
		return false;
	}
	if (Uniform) {
		auto screen = math::fvec2(Graphic::GetRenderDevice()->GetContextSize());
		float Aspect = screen[0] / screen[1];
		FrameSize = m_Texture->GetSize();
		FrameSize /= math::vec2(FrameStripCount);
		FrameSize /= screen;
		FrameSize.x *= Aspect;
		FrameSize *= 2.0f;
		m_FrameSize = FrameSize;
	} else {
		if (FrameCount == 1 && (FrameSize[0] == 0 || FrameSize[1] == 0)) {
			FrameSize = math::vec2(m_Texture->GetSize());
		}
	}

	if (!GenerateFrames(FrameSize, FrameStripCount)) {
		AddLog(Error, "Unable to frames for animation!");
		return false;
	}

	Graphic::GetRenderDevice()->RequestContextManip([this]() {
		m_DrawEnabled = true;
		AddLog(Debug, "Animation '" << this->GetName() << "' has been loaded");
	});

	return true;
}

bool Animation::GenerateFrames(math::vec2 FrameSize, math::vec2 FrameStripCount) {
	unsigned FrameCount = m_EndFrame - m_StartFrame + 1;
	m_FrameTable.reset(new Graphic::VAO[FrameCount]);

	math::vec2 texsize = m_Texture->GetSize();

	math::vec2 fu = math::vec2(1.0f) / FrameStripCount;

	for (unsigned y = 0; y < m_FrameCount[1]; ++y)
	for (unsigned x = 0; x < m_FrameCount[0]; ++x) {
		unsigned frame = y * m_FrameCount[0] + x;
		if (frame > FrameCount)
			continue;

		Graphic::VertexVector Vertexes{
			Graphic::vec3(0, FrameSize[1], 0),
			Graphic::vec3(FrameSize[0], FrameSize[1], 0),
			Graphic::vec3(FrameSize[0], 0, 0),
			Graphic::vec3(0, 0, 0),
		};
		Graphic::NormalVector Normals;
		float w1 = fu[0] * (float)x;
		float h1 = fu[1] * (float)y;
		float w2 = w1 + fu[0];
		float h2 = h1 + fu[1];
		Graphic::TexCoordVector TexUV{
			Graphic::vec2(w1, h1),
			Graphic::vec2(w2, h1),
			Graphic::vec2(w2, h2),
			Graphic::vec2(w1, h2),
		};

		Graphic::IndexVector Index{ 0, 1, 2, 3 };
		m_FrameTable[frame].DelayInit(Vertexes, TexUV, Normals, Index);
	}

	return true;
}

//----------------------------------------------------------------

AnimationInstance Animation::CreateInstance() {
	AnimationInstance ai;
	ai.Data = shared_from_this();
	ai.Position = static_cast<float>(m_StartFrame);
	return ai;
}
 
void Animation::UpdateInstance(const Core::MoveConfig &conf, AnimationInstance &instance) {
	if (m_Speed > 0) {
		instance.Position += m_Speed * conf.TimeDelta;
		if (instance.Position > m_EndFrame) {
			auto delta = m_EndFrame - m_StartFrame;
			int mult = static_cast<int>(instance.Position / delta);
			instance.Position -= static_cast<float>(mult) * delta;
		} 
		//else
			//if (instance.Position < m_StartFrame) {
				//auto delta = m_EndFrame - m_StartFrame;
				//int mult = static_cast<int>(instance.Position / delta);
			//}
	}
}

void Animation::Draw(Graphic::MatrixStack &dev, AnimationInstance &instance) const {
	return Draw(static_cast<unsigned>(instance.Position));
}

void Animation::Draw(unsigned Frame) const {
	if (!m_DrawEnabled) return;
	if (Frame > m_EndFrame)
		Frame = m_EndFrame;
	else
		if (Frame < m_StartFrame)
			Frame = m_StartFrame;

	m_Texture->Bind();
	auto &vao = m_FrameTable[Frame];
	vao.Bind();
	vao.DrawElements(4, 0, 0, GL_QUADS);
	vao.UnBind();
}

} //namespace GUI 
} //namespace MoonGlare 

