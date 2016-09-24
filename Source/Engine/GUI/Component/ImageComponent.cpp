/*
  * Generated by cppsrc.sh
  * On 2016-09-18 19:00:53,20
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "../nfGUI.h"

#include <Core/Component/ComponentManager.h>
#include <Core/Component/ComponentRegister.h>
#include "RectTransformComponent.h"
#include "ImageComponent.h"

#include <Renderer/Commands/ControllCommands.h>
#include <Renderer/Commands/ShaderCommands.h>
#include <Renderer/Commands/TextureCommands.h>
#include <Renderer/Commands/ArrayCommands.h>
#include <Renderer/RenderInput.h>

#include <Math.x2c.h>
#include <ComponentCommon.x2c.h>
#include <ImageComponent.x2c.h>

namespace MoonGlare {
namespace GUI {
namespace Component {

//---------------------------------------------------------------------------------------

struct ImageShader : public ::Graphic::Shader {
	ImageShader(GLuint ShaderProgram, const std::string &ProgramName) : ::Graphic::Shader(ShaderProgram, ProgramName) {
		m_BaseColorLocation = Location("gBaseColor");
	}

	GLint m_BaseColorLocation;

	void Bind(Renderer::CommandQueue &Queue, Renderer::RendererConf::CommandKey key) {
		Queue.PushCommand<Renderer::Commands::ShaderBind>(key)->m_Shader = Handle();
	}
	void SetWorldMatrix(Renderer::CommandQueue &Queue, Renderer::RendererConf::CommandKey key, const glm::mat4 & ModelMat, const glm::mat4 &CameraMat) {
		auto loc = Location(ShaderParameters::WorldMatrix);
		if (!IsValidLocation(loc))
			return;

		auto arg = Queue.PushCommand<Renderer::Commands::ShaderSetUniformMatrix4>(key);
		arg->m_Location = loc;
		arg->m_Matrix = CameraMat * ModelMat;
	}
	void SetColor(Renderer::CommandQueue &Queue, Renderer::RendererConf::CommandKey key, const math::vec4 &color) {
		if (!IsValidLocation(m_BaseColorLocation))
			return;

		auto arg = Queue.PushCommand<Renderer::Commands::ShaderSetUniformVec4>(key);
		arg->m_Location = m_BaseColorLocation;
		arg->m_Vec = color;
	}

	//void VAORelease(Renderer::CommandQueue &Queue) {
	//	Queue.PushCommand<Renderer::Commands::VAORelease>();
	//}

	void Enable(Renderer::CommandQueue &Queue, GLenum what) {
		Queue.PushCommand<Renderer::Commands::Enable>(Renderer::RendererConf::CommandKey{ 0 })->m_What = what;
	}
	void Disable(Renderer::CommandQueue &Queue, GLenum what) {
		Queue.PushCommand<Renderer::Commands::Disable>(Renderer::RendererConf::CommandKey{ 0 })->m_What = what;
	}
};

//---------------------------------------------------------------------------------------

::Space::RTTI::TypeInfoInitializer<ImageComponent, ImageComponentEntry> ImageComponentTypeInfo;
RegisterComponentID<ImageComponent> ImageComponentIDReg("Image", true, &ImageComponent::RegisterScriptApi);

//---------------------------------------------------------------------------------------

ImageComponent::ImageComponent(ComponentManager *Owner) 
		: TemplateStandardComponent(Owner)
{
	m_RectTransform = nullptr;
	m_Shader = nullptr;
}

ImageComponent::~ImageComponent() {
}

//---------------------------------------------------------------------------------------

void ImageComponent::RegisterScriptApi(ApiInitializer & root) {
}

//---------------------------------------------------------------------------------------

bool ImageComponent::Initialize() {
	//memset(&m_Array, 0, m_Array.Capacity() * sizeof(m_Array[0]));
	m_Array.fill(ImageComponentEntry());
	m_Array.ClearAllocation();

	m_RectTransform = GetManager()->GetComponent<RectTransformComponent>();
	if (!m_RectTransform) {
		AddLog(Error, "Failed to get RectTransformComponent instance!");
		return false;
	}

	return true;
}

bool ImageComponent::Finalize() {
	return true;
}

//---------------------------------------------------------------------------------------

void ImageComponent::Step(const Core::MoveConfig & conf) {
	auto &Queue = conf.m_RenderInput->m_CommandQueues[(size_t)Renderer::CommandQueueID::GUI];
	auto QueueSavePoint = Queue.GetSavePoint();
	bool QueueDirty = false;
	bool CanRender = false;

	if (!m_Shader) {
		if (!Graphic::GetShaderMgr()->GetSpecialShaderType<ImageShader>("GUI.Image", m_Shader)) {
			AddLogf(Error, "Failed to load GUI.Image shader");
			return;
		}
	}

	if (m_Shader) {

		m_Shader->Enable(Queue, GL_BLEND);
		m_Shader->Enable(Queue, GL_DEPTH_TEST);
		m_Shader->Disable(Queue, GL_CULL_FACE);

		CanRender = true;
	}

	size_t LastInvalidEntry = 0;
	size_t InvalidEntryCount = 0;

	for (size_t i = 0; i < m_Array.Allocated(); ++i) {
		auto &item = m_Array[i];

		if (!item.m_Flags.m_Map.m_Valid) {
			//mark and continue
			LastInvalidEntry = i;
			++InvalidEntryCount;
			continue;
		}

		auto *rtentry = m_RectTransform->GetEntry(item.m_OwnerEntity);
		if (!rtentry) {
			LastInvalidEntry = i;
			++InvalidEntryCount;
			continue;
		}

		item.Update(conf.TimeDelta, *rtentry);
		
		if (!item.m_Animation || !item.m_Flags.m_Map.m_Visible)
			continue;

		if (!CanRender)
			continue;
		
		Renderer::RendererConf::CommandKey key{ rtentry->m_Z };

		m_Shader->Bind(Queue, key);

		m_Shader->SetWorldMatrix(Queue, key, item.m_ImageMatrix, m_RectTransform->GetCamera().GetProjectionMatrix());
		m_Shader->SetColor(Queue, key, item.m_Color);

		Queue.PushCommand<Renderer::Commands::Texture2DBind>(key)->m_Texture = item.m_Animation->GetTexture()->Handle();
		Queue.PushCommand<Renderer::Commands::VAOBind>(key)->m_VAO = item.m_Animation->GetFrameVAO(static_cast<unsigned>(item.m_Position)).Handle();

		auto arg = Queue.PushCommand<Renderer::Commands::VAODrawTriangles>(key);
		arg->m_NumIndices = 6;
		arg->m_IndexValueType = GL_UNSIGNED_INT;

		QueueDirty = true;
	}

	if (!QueueDirty)
		Queue.Rollback(QueueSavePoint);
	else {
		//	m_Shader->VAORelease(Queue);
	}

	if (InvalidEntryCount > 0) {
		AddLogf(Performance, "TransformComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
		TrivialReleaseElement(LastInvalidEntry);
	}
}

//---------------------------------------------------------------------------------------

bool ImageComponent::Load(xml_node node, Entity Owner, Handle & hout) {
	size_t index;
	if (!m_Array.Allocate(index)) {
		AddLogf(Error, "Failed to allocate index!");
		return false;
	}
	auto &entry = m_Array[index];
	entry.Reset();
	if (!GetHandleTable()->Allocate(this, Owner, entry.m_SelfHandle, index)) {
		AddLog(Error, "Failed to allocate handle");
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}
	hout = entry.m_SelfHandle;
	entry.m_OwnerEntity = Owner;

	x2c::Component::ImageComponent::ImageEntry_t ie;
	ie.ResetToDefault();
	if (!ie.Read(node)) {
		AddLog(Error, "Failed to read ImageEntry!");
		return false;
	}

	entry.m_Animation = std::make_shared<Animation>();
	entry.m_Speed = ie.m_Speed;
	entry.m_Position = 0.0f;
	entry.m_FrameCount = ie.m_FrameCount;
	entry.m_Flags.m_Map.m_Visible = ie.m_Visible;
	entry.m_ScaleMode = ie.m_ScaleMode;
	entry.m_Color = ie.m_Color;

	entry.m_Animation->Load(ie.m_TextureURI, ie.m_StartFrame, ie.m_FrameCount, ie.m_FrameStripCount, ie.m_Spacing, ie.m_FrameSize, m_RectTransform->IsUniformMode());

	auto *rtentry = m_RectTransform->GetEntry(entry.m_OwnerEntity);
	if (rtentry) {
		entry.Update(0.0f, *rtentry);
	} else {
		//TODO:??
	}

	entry.m_Flags.m_Map.m_Valid = true;
	entry.m_Flags.m_Map.m_Dirty = true;
	m_EntityMapper.SetComponentMapping(entry);
	return true;
}

//---------------------------------------------------------------------------------------

void ImageComponentEntry::Update(float TimeDelta, RectTransformComponentEntry &rectTransform) {
	if (m_Speed > 0) {
		m_Position += m_Speed * TimeDelta;
		if ((unsigned)m_Position >= m_FrameCount) {
			int mult = static_cast<int>(m_Position / m_FrameCount);
			m_Position -= static_cast<float>(mult) * m_FrameCount;
		}
		//else
		//if (instance.Position < m_StartFrame) {
		//auto delta = m_EndFrame - m_StartFrame;
		//int mult = static_cast<int>(instance.Position / delta);
		//}
	}

	if (m_Flags.m_Map.m_Dirty || m_TransformRevision != rectTransform.m_Revision) {
		m_Flags.m_Map.m_Dirty = false;
		m_TransformRevision = rectTransform.m_Revision;

		math::vec3 Pos, Scale;

		switch (m_ScaleMode) {
		case ImageScaleMode::None:
			m_ImageMatrix = rectTransform.m_GlobalMatrix;
			return;
		case ImageScaleMode::Center: {
			auto halfparent = rectTransform.m_ScreenRect.GetSize() / 2.0f;
			auto halfsize = m_Animation->GetFrameSize() / 2.0f;
			Pos = math::vec3(halfparent - halfsize, 0.0f);
			Scale = math::vec3(1.0f);
			break;
		}
		case ImageScaleMode::ScaleToFit:{
			Point ratio = rectTransform.m_ScreenRect.GetSize() / m_Animation->GetFrameSize();
			Scale = math::vec3(ratio, 1.0f);
			Pos = math::vec3(0);
			break;
		}
		case ImageScaleMode::ScaleProportional: 
		case ImageScaleMode::ScaleProportionalCenter: {
			auto parentsize = rectTransform.m_ScreenRect.GetSize();
			auto &framesize = m_Animation->GetFrameSize();
			Point ratio = parentsize / framesize;
			float minration = math::min(ratio.x, ratio.y);
			Scale = math::vec3(minration, minration, 1.0f);
			if (m_ScaleMode == ImageScaleMode::ScaleProportionalCenter) {
				auto halfparent = rectTransform.m_ScreenRect.GetSize() / 2.0f;
				auto halfsize = (m_Animation->GetFrameSize() * math::vec2(minration)) / 2.0f;
				Pos = math::vec3(halfparent - halfsize, 0.0f);
			} else {
				Pos = math::vec3(0.0f);
			}
			break;
		}
		default:
			LogInvalidEnum(m_ScaleMode);
			return;
		}
		m_ImageMatrix = rectTransform.m_GlobalMatrix * glm::scale(glm::translate(math::mat4(), Pos), Scale);
	}
}

} //namespace Component 
} //namespace GUI 
} //namespace MoonGlare 
