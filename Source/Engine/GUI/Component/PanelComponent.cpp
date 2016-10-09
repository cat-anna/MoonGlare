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
#include "PanelComponent.h"

#include <Renderer/Commands/ControllCommands.h>
#include <Renderer/Commands/ShaderCommands.h>
#include <Renderer/Commands/TextureCommands.h>
#include <Renderer/Commands/ArrayCommands.h>
#include <Renderer/RenderInput.h>
#include "../GUIShader.h"

#include <Math.x2c.h>
#include <ComponentCommon.x2c.h>
#include <RectTransformComponent.x2c.h>
#include <PanelComponent.x2c.h>

namespace MoonGlare {
namespace GUI {
namespace Component {

//---------------------------------------------------------------------------------------

::Space::RTTI::TypeInfoInitializer<PanelComponent, PanelComponentEntry> PanelComponentTypeInfo;
RegisterComponentID<PanelComponent> PanelComponentIDReg("Panel", true, &PanelComponent::RegisterScriptApi);

//---------------------------------------------------------------------------------------

PanelComponent::PanelComponent(ComponentManager *Owner) 
		: TemplateStandardComponent(Owner)
{
	m_RectTransform = nullptr;
	m_Shader = nullptr;
}

PanelComponent::~PanelComponent() {
}

//---------------------------------------------------------------------------------------

void PanelComponent::RegisterScriptApi(ApiInitializer & root) {
	root
		.beginClass<PanelComponentEntry>("cPanelComponentEntry")
			.addProperty("Color", &PanelComponentEntry::GetColor, &PanelComponentEntry::SetColor)
			.addProperty("Border", &PanelComponentEntry::GetBorder, &PanelComponentEntry::SetBorder)
			.addProperty("TileMode", &PanelComponentEntry::GetTileMode, &PanelComponentEntry::SetTileMode)
		.endClass()
		;
}

//---------------------------------------------------------------------------------------

bool PanelComponent::Initialize() {
	//memset(&m_Array, 0, m_Array.Capacity() * sizeof(m_Array[0]));
//	m_Array.fill(PanelComponentEntry());
	m_Array.ClearAllocation();

	m_RectTransform = GetManager()->GetComponent<RectTransformComponent>();
	if (!m_RectTransform) {
		AddLog(Error, "Failed to get RectTransformComponent instance!");
		return false;
	}

	::Graphic::GetRenderDevice()->RequestContextManip([this]() {
		if (!m_Shader) {
			if (!Graphic::GetShaderMgr()->GetSpecialShaderType<GUIShader>("GUI.Panel", m_Shader)) {
				AddLogf(Error, "Failed to load GUI.Panel shader");
			}
		}
	});

	return true;
}

bool PanelComponent::Finalize() {
	return true;
}

//---------------------------------------------------------------------------------------

void PanelComponent::Step(const Core::MoveConfig & conf) {
	size_t LastInvalidEntry = 0;
	size_t InvalidEntryCount = 0;

	auto &Queue = conf.m_RenderInput->m_CommandQueues[(size_t)Renderer::CommandQueueID::GUI];
	auto QueueSavePoint = Queue.GetSavePoint();
	bool QueueDirty = false;
	bool CanRender = false;

	if (m_Shader) {
		CanRender = true;
	}

	for (size_t i = 0; i < m_Array.Allocated(); ++i) {
		auto &item = m_Array[i];

		if (!item.m_Flags.m_Map.m_Valid) {
			//mark and continue
			LastInvalidEntry = i;
			++InvalidEntryCount;
			continue;
		}

		if (!GetHandleTable()->IsValid(this, item.m_SelfHandle)) {
			item.m_Flags.m_Map.m_Valid = false;
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

		if (!item.m_Flags.m_Map.m_Active)
			continue;

		if (item.m_TransformRevision == rtentry->m_Revision && !item.m_Flags.m_Map.m_Dirty) {
		} else {

			item.m_Flags.m_Map.m_Dirty = false;
			item.m_TransformRevision = rtentry->m_Revision;

			auto size = rtentry->m_ScreenRect.GetSize();

			Graphic::VertexVector Vertexes{
				Graphic::vec3(0, size[1], 0),
				Graphic::vec3(size[0], size[1], 0),
				Graphic::vec3(size[0], 0, 0),
				Graphic::vec3(0, 0, 0),
			};
			Graphic::NormalVector Normals;
			float w1 = 0.0f;
			float h1 = 0.0f;
			float w2 = 1.0f;
			float h2 = 1.0f;
			Graphic::TexCoordVector TexUV{
				Graphic::vec2(w1, h1),
				Graphic::vec2(w2, h1),
				Graphic::vec2(w2, h2),
				Graphic::vec2(w1, h2),
			};
			Graphic::IndexVector Index{ 0, 1, 2, 0, 2, 3, };
			item.m_VAO.DelayInit(Vertexes, TexUV, Normals, Index);
		}

		if (!CanRender)
			continue;

		Renderer::RendererConf::CommandKey key{ rtentry->m_Z };

		m_Shader->SetWorldMatrix(Queue, key, rtentry->m_GlobalMatrix, m_RectTransform->GetCamera().GetProjectionMatrix());
		
		m_Shader->SetPanelSize(Queue, key, rtentry->m_ScreenRect.GetSize());
		m_Shader->SetBorder(Queue, key, item.m_Border);
		m_Shader->SetColor(Queue, key, item.m_Color);
		m_Shader->SetTileMode(Queue, key, item.m_TileMode);

		Queue.PushCommand<Renderer::Commands::Texture2DBind>(key)->m_Texture = item.m_Texture->Handle();
		Queue.PushCommand<Renderer::Commands::VAOBind>(key)->m_VAO = item.m_VAO.Handle();

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

bool PanelComponent::Load(xml_node node, Entity Owner, Handle & hout) {
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

	x2c::Component::PanelComponent::PanelEntry_t pe;
	pe.ResetToDefault();
	if (!pe.Read(node)) {
		AddLog(Error, "Failed to read ImageEntry!");
		return false;
	}

	auto *rtentry = m_RectTransform->GetEntry(entry.m_OwnerEntity);
	if (rtentry) {
	} else {
		//TODO:??
	}

	if (!GetFileSystem()->OpenTexture(entry.m_Texture, pe.m_TextureURI)) {
		AddLog(Error, "Unable to load texture file for panel!");
		return false;
	}
	
	entry.m_Border = pe.m_Border;
	entry.m_Color = pe.m_Color;
	entry.m_TileMode = pe.m_TileMode;
	entry.m_Flags.m_Map.m_Active = pe.m_Active;

	entry.m_Flags.m_Map.m_Valid = true;
	entry.m_Flags.m_Map.m_Dirty = true;
	m_EntityMapper.SetComponentMapping(entry);
	return true;
}

//---------------------------------------------------------------------------------------

} //namespace Component 
} //namespace GUI 
} //namespace MoonGlare 
