/*
	Generated by cppsrc.sh
	On 2015-01-12 15:56:50,19
	by Paweu
*/

#pragma once
#ifndef Environment_H
#define Environment_H

namespace Graphic {

//! This class holds information about scene environment.
class Environment : public cRootClass {
	SPACERTTI_DECLARE_CLASS_NOCREATOR(Environment, cRootClass);
public:
	Environment();
	~Environment();

	//! Load metedata of environment from xml
	bool LoadMeta(const xml_node node);

	//! Bind environment settings to shader
	void Bind(Shader *s) const {
		s->Bind(&m_StaticFog);
	}

	//! Render environment
	void Render(cRenderDevice &dev) const {
		if (m_HaveSkyCube)
			m_SkyCube.Render(dev);
	}

	
	/** Initialize environment.*/
	bool Initialize();
	/** Finalize environment.*/
	bool Finalize();
protected:
	StaticFog m_StaticFog;

	bool m_HaveSkyCube;
	SkyCube m_SkyCube;
};

} //namespace Graphic 

#endif

