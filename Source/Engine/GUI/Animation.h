/*
  * Generated by cppsrc.sh
  * On 2015-03-06  9:44:21,50
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#ifndef GUI_Texture_H
#define GUI_Texture_H

namespace MoonGlare {
namespace GUI {
	
class Animation : public cRootClass, public std::enable_shared_from_this<Animation> {
	SPACERTTI_DECLARE_STATIC_CLASS(Animation, cRootClass);
public:
 	Animation();
 	~Animation();

	DefineRefGetterConst(Name, string);

	bool LoadMeta(const xml_node node);
	bool Load(const std::string &fileuri, unsigned StartFrame, unsigned FrameCount, math::uvec2 FrameStripCount, math::uvec2 Spacing, math::vec2 FrameSize, bool Uniform);

	AnimationInstance CreateInstance();
	void UpdateInstance(const Core::MoveConfig &conf, AnimationInstance &instance);

	void Draw(Graphic::MatrixStack &dev, AnimationInstance &instance) const;
	void Draw(unsigned Frame) const;

	DefineRefGetterConst(FrameSize, math::vec2);
protected:
	string m_Name;
	float m_Speed;
	TextureFile m_Texture;
	unsigned m_StartFrame, m_EndFrame;
	math::vec2 m_FrameSize;
	math::uvec2 m_FrameSpacing;
	math::uvec2 m_FrameCount;
	std::unique_ptr<Graphic::VAO[]> m_FrameTable;
	bool m_DrawEnabled;

	bool GenerateFrames(math::vec2 FrameSize, math::vec2 FrameStripCount);
};

struct AnimationInstance {
	SharedAnimation Data;
	float Position;

	void Update(const Core::MoveConfig &conf) { if(Data) Data->UpdateInstance(conf, *this); }
	void Draw(Graphic::MatrixStack &dev) { if(Data) Data->Draw(dev, *this); }
};

} //namespace GUI 
} //namespace MoonGlare 

#endif
