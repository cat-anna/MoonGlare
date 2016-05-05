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
		m_FrameSize(16, 16),
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
	auto nodeFrames = node.child("Frames");
	m_StartFrame = nodeFrames.attribute("Start").as_uint(m_StartFrame);
	unsigned FrameCount = nodeFrames.attribute("Count").as_uint(1);
	m_EndFrame = m_StartFrame + FrameCount ;

	auto nodeStrip = node.child("Strip");
	XML::Vector::Read(nodeStrip, "Size", m_FrameSize);
	XML::Vector::Read(nodeStrip, "Spacing", m_FrameSpacing);
	XML::Vector::Read(nodeStrip, "Count", m_FrameCount, math::uvec2(1, 1));

	string tex = node.child("Texture").text().as_string(ERROR_STR);
	if (!GetFileSystem()->OpenTexture(m_Texture, tex, DataPath::Texture, true)) {
		AddLog(Error, "Unable to load texture file for animation!");
		return false;
	}

	if (FrameCount == 0) {
		FrameCount = 1;
		m_StartFrame = m_EndFrame = 0;
	}

	if (FrameCount == 1 && (m_FrameSize[0] == 0 || m_FrameSize[1] == 0)) {
		m_FrameSize = m_Texture->GetSize();
	}

	if (!GenerateFrames()) {
		AddLog(Error, "Unable to frames for animation!");
		return false;
	}

	Graphic::GetRenderDevice()->RequestContextManip([this]() {
		m_DrawEnabled = true;
		AddLog(Debug, "Animation '" << this->GetName() << "' has been loaded");
	});

	return true;
}

bool Animation::GenerateFrames() {
	unsigned FrameCount = m_EndFrame - m_StartFrame;
	m_FrameTable.reset(new Graphic::VAO[FrameCount]);

	math::vec2 texsize = m_Texture->GetSize();
	math::vec2 framesize = m_FrameSize;

	math::vec2 fu;
	if (FrameCount > 1)
		fu = framesize / texsize;
	else
		fu = math::vec2(1.0f);

	for (unsigned i = 0; i < m_FrameCount[0]; ++i)
	for (unsigned j = 0; j < m_FrameCount[1]; ++j) {
		Graphic::VertexVector Vertexes{
			Graphic::vec3(0, framesize[1], 0),
			Graphic::vec3(framesize[0], framesize[1], 0),
			Graphic::vec3(framesize[0], 0, 0),
			Graphic::vec3(0, 0, 0),
		};
		Graphic::NormalVector Normals;
		float w1 = fu[0] * (float)j;
		float h1 = fu[1] * (float)i;
		float w2 = w1 + fu[0];
		float h2 = h1 + fu[1];
		Graphic::TexCoordVector TexUV{
			Graphic::vec2(w1, h1),
			Graphic::vec2(w2, h1),
			Graphic::vec2(w2, h2),
			Graphic::vec2(w1, h2),
		};

		Graphic::IndexVector Index{ 0, 1, 2, 3 };

		unsigned frame = i * m_FrameCount[0] + j;
		if (frame > FrameCount)
			break;
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
	if (!m_DrawEnabled) return;
	unsigned frame = static_cast<unsigned>(instance.Position);
	if (frame > m_EndFrame)
		frame = m_EndFrame;
	else
		if (frame < m_StartFrame)
			frame = m_StartFrame;

	m_Texture->Bind();
	auto &vao = m_FrameTable[frame];
	vao.Bind();
	vao.DrawElements(4, 0, 0, GL_QUADS);
	vao.UnBind();
}

} //namespace GUI 
} //namespace MoonGlare 

