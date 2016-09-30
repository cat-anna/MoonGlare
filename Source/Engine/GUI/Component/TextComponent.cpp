/*
  * Generated by cppsrc.sh
  * On 2016-09-23 21:24:39,56
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include <Engine/DataClasses/iFont.h>
#include "../nfGUI.h"

#include <Core/Component/ComponentManager.h>
#include <Core/Component/ComponentRegister.h>
#include "RectTransformComponent.h"
#include "TextComponent.h"

#include <Renderer/Commands/ControllCommands.h>
#include <Renderer/Commands/ShaderCommands.h>
#include <Renderer/Commands/TextureCommands.h>
#include <Renderer/Commands/ArrayCommands.h>
#include <Renderer/RenderInput.h>
#include "../GUIShader.h"

#include <Math.x2c.h>
#include <ComponentCommon.x2c.h>
#include <TextComponent.x2c.h>

namespace MoonGlare {
namespace GUI {
namespace Component {

//---------------------------------------------------------------------------------------

::Space::RTTI::TypeInfoInitializer<TextComponent, TextComponentEntry> TextComponentTypeInfo;
RegisterComponentID<TextComponent>TextComponentIDReg("Text", true, &TextComponent::RegisterScriptApi);

//---------------------------------------------------------------------------------------

TextComponent::TextComponent(ComponentManager * Owner)
		: TemplateStandardComponent(Owner) {
	m_RectTransform = nullptr;
	m_Shader = nullptr;
}

TextComponent::~TextComponent() {
}

//---------------------------------------------------------------------------------------

void TextComponent::RegisterScriptApi(ApiInitializer & root) {
	root
		.beginClass<TextComponentEntry>("cTextComponentEntry")
			.addProperty("Color", &TextComponentEntry::GetColor, &TextComponentEntry::SetColor)
			.addProperty("Text", &TextComponentEntry::GetText, &TextComponentEntry::SetText)
		.endClass()
		; 
}

//---------------------------------------------------------------------------------------

bool TextComponent::Initialize() {
	//memset(&m_Array, 0, m_Array.Capacity() * sizeof(m_Array[0]));
	m_Array.fill(TextComponentEntry());
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

bool TextComponent::Finalize() {
	return true;
}

//---------------------------------------------------------------------------------------

void TextComponent::Step(const Core::MoveConfig & conf) {
	auto &Queue = conf.m_RenderInput->m_CommandQueues[(size_t)Renderer::CommandQueueID::GUI];
	auto QueueSavePoint = Queue.GetSavePoint();
	bool QueueDirty = false;
	bool CanRender = false;

	if (m_Shader) {
		CanRender = true;
	}

	size_t LastInvalidEntry = 0;
	size_t InvalidEntryCount = 0;

	for (size_t i = 0; i < m_Array.Allocated(); ++i) {
		auto &entry = m_Array[i];

		if (!entry.m_Flags.m_Map.m_Valid) {
			//mark and continue
			LastInvalidEntry = i;
			++InvalidEntryCount;
			continue;
		}

		auto *rtentry = m_RectTransform->GetEntry(entry.m_OwnerEntity);
		if (!rtentry) {
			LastInvalidEntry = i;
			++InvalidEntryCount;
			continue;
		}

		if (!entry.m_Flags.m_Map.m_Active)
			continue;

		if (entry.m_Flags.m_Map.m_Dirty) {
			std::wstring txt = Utils::Strings::towstring(entry.m_Text);
			entry.m_FontInstance = DataClasses::SharedFontInstance(entry.m_Font->GenerateInstance(txt.c_str(), &entry.m_FontStyle, m_RectTransform->IsUniformMode()).release());
		
			entry.m_Flags.m_Map.m_Dirty = false;
		}

		if (!entry.m_FontInstance)
			continue;
		if (!CanRender)
			continue;

		Renderer::RendererConf::CommandKey key{ rtentry->m_Z };
		m_Shader->SetWorldMatrix(Queue, key, rtentry->m_GlobalMatrix, m_RectTransform->GetCamera().GetProjectionMatrix());
		m_Shader->SetColor(Queue, key, math::vec4(entry.m_FontStyle.Color, 1.0f));
		m_Shader->SetTileMode(Queue, key, math::vec2(0, 0));
		entry.m_FontInstance->GenerateCommands(Queue, rtentry->m_Z);

//		Queue.PushCommand<Renderer::Commands::Texture2DBind>(key)->m_Texture = item.m_Animation->GetTexture()->Handle();
//		Queue.PushCommand<Renderer::Commands::VAOBind>(key)->m_VAO = item.m_Animation->GetFrameVAO(static_cast<unsigned>(item.m_Position)).Handle();
//		auto arg = Queue.PushCommand<Renderer::Commands::VAODrawTriangles>(key);
//		arg->m_NumIndices = 6;
//		arg->m_IndexValueType = GL_UNSIGNED_INT;

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

bool TextComponent::Load(xml_node node, Entity Owner, Handle & hout) {
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

	x2c::Component::TextComponent::TextEntry_t te;
	te.ResetToDefault();
	if (!te.Read(node)) {
		AddLog(Error, "Failed to read ImageEntry!");
		return false;
	}

	entry.m_Font = GetDataMgr()->GetFont(te.m_FontName);
	if (!entry.m_Font) {
		AddLogf(Error, "Unknown font: %s", te.m_FontName.c_str());
		return false;
	}

	entry.m_FontStyle.Size = te.m_FontSize;
	entry.m_FontStyle.Color = math::vec3(1, 1, 1);
	entry.m_Text = te.m_Text;
	entry.m_Flags.m_Map.m_Active = te.m_Active;

	entry.m_Flags.m_Map.m_Valid = true;
	entry.m_Flags.m_Map.m_Dirty = true;
	m_EntityMapper.SetComponentMapping(entry);
	return true;
}

//---------------------------------------------------------------------------------------

} //namespace Component 
} //namespace GUI 
} //namespace MoonGlare 
