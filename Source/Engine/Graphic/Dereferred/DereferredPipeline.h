/*
	Generated by cppsrc.sh
	On 2015-01-14 22:33:57,87
	by Paweu
*/

#pragma once
#ifndef DerefferedPipeline_H
#define DerefferedPipeline_H

#include "DereferredFrameBuffer.h"
#include "../PlaneShadowMap.h"

#include <libSpace/src/Container/StaticVector.h>

namespace Graphic {
namespace Dereferred {

class DereferredPipeline : public cRootClass {
	SPACERTTI_DECLARE_CLASS_NOCREATOR(DereferredPipeline, cRootClass);
public:
 	DereferredPipeline();
 	virtual ~DereferredPipeline();

	bool Initialize();
	bool Finalize();

	bool Execute(const MoonGlare::Core::MoveConfig &conf, cRenderDevice& dev);

	void BeginFrame(cRenderDevice& dev);
	bool RenderShadows(RenderInput *ri, cRenderDevice& dev);
	bool RenderGeometry(RenderInput *ri, cRenderDevice& dev);
	bool RenderLights(RenderInput *ri, cRenderDevice& dev);

	//bool RenderPointLightsShadows(Core::ciScene *scene, Light::PointLightList &lights, cRenderDevice& dev);
	bool RenderSpotLightsShadows(RenderInput *ri, cRenderDevice& dev);

	bool RenderPointLights(RenderInput *ri, cRenderDevice& dev);
	bool RenderDirectionalLights(RenderInput *ri, cRenderDevice& dev);
	bool RenderSpotLights(RenderInput *ri, cRenderDevice& dev);

	void FinalPass(const uvec2 &size);
	void EndFrame();
	DereferredFrameBuffer m_Buffer;

	struct Flags {
		enum {
			Ready		= 1,
		};
	};

	DefineFlagGetter(m_Flags, Flags::Ready, Ready);
private: 
	unsigned m_Flags;
	Shader *m_StencilShader;
	Shaders::ShadowMapShader *m_ShadowMapShader;
	GeometryPassShader *m_GeometryShader;
	PointLightShader *m_PointLightShader;
	DirectionalLightShader *m_DirectionalLightShader;
	SpotLightShader *m_SpotLightShader;
	
	MoonGlare::DataClasses::ModelPtr m_Sphere, m_Cone;
	VAO m_DirectionalQuad;

	Space::Container::StaticVector<PlaneShadowMap, 1024> m_PlaneShadowMapBuffer;

	DefineFlagSetter(m_Flags, Flags::Ready, Ready);

	bool InitializeDirectionalQuad();
};

} //namespace Dereferred
} //namespace Graphic 

#endif
