/*
  * Generated by cppsrc.sh
  * On 2016-09-12 17:58:33,31
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "../nfGUI.h"

#include <Core/Component/ComponentManager.h>
#include <Core/Component/ComponentRegister.h>
#include "RectTransformComponent.h"

#include <Renderer/RenderInput.h>

#include <Math.x2c.h>
#include <ComponentCommon.x2c.h>
#include <RectTransformComponent.x2c.h>

namespace MoonGlare {
namespace GUI {
namespace Component {

::Space::RTTI::TypeInfoInitializer<RectTransformComponent, RectTransformComponentEntry> RectTransformComponentTypeInfo;
RegisterComponentID<RectTransformComponent> RectTransformComponentIDReg("RectTransform", true, &RectTransformComponent::RegisterScriptApi);

RectTransformComponent::RectTransformComponent(ComponentManager *Owner)
	: TemplateStandardComponent(Owner) {
}

RectTransformComponent::~RectTransformComponent() {
}

//---------------------------------------------------------------------------------------

void RectTransformComponent::RegisterScriptApi(ApiInitializer & root) {
	root
		.beginClass<RectTransformComponentEntry>("cRectTransformComponentEntry")
			.addProperty("Position", &RectTransformComponentEntry::GetPosition, &RectTransformComponentEntry::SetPosition)
			.addProperty("Size", &RectTransformComponentEntry::GetSize, &RectTransformComponentEntry::SetSize)
			.addProperty("Z", &RectTransformComponentEntry::GetZ, &RectTransformComponentEntry::SetZ)
		.endClass()
		;
}

//---------------------------------------------------------------------------------------

bool RectTransformComponent::Initialize() {
	memset(&m_Array, 0, m_Array.Capacity() * sizeof(m_Array[0]));
	m_CurrentRevision = 1;

	m_Array.ClearAllocation();
	size_t index;
	if (!m_Array.Allocate(index)) {
		AddLogf(Error, "Failed to allocate index!");
		return false;
	}

	auto &RootEntry = m_Array[index];
	RootEntry.m_Flags.ClearAll();
	RootEntry.m_Flags.m_Map.m_Valid = true;
	auto *EntityManager = GetManager()->GetWorld()->GetEntityManager();
	RootEntry.m_OwnerEntity = EntityManager->GetRootEntity();

	m_ScreenSize = math::fvec2(Graphic::GetRenderDevice()->GetContextSize());

	if (m_Flags.m_Map.m_UniformMode) {
		float Aspect = m_ScreenSize[0] / m_ScreenSize[1];
		RootEntry.m_ScreenRect.LeftTop = Point(-Aspect, -1.0f);
		RootEntry.m_ScreenRect.RightBottom = -RootEntry.m_ScreenRect.LeftTop;
	} else {
		RootEntry.m_ScreenRect.LeftTop = Point(0,0);
		RootEntry.m_ScreenRect.RightBottom = m_ScreenSize;
	}

	RootEntry.m_Position = RootEntry.m_ScreenRect.LeftTop;
	RootEntry.m_Size = RootEntry.m_ScreenRect.GetSize();
	RootEntry.m_GlobalMatrix = glm::translate(math::mat4(), math::vec3(RootEntry.m_ScreenRect.LeftTop, 1.0f));
	RootEntry.m_LocalMatrix = math::mat4();

	auto &rb = RootEntry.m_ScreenRect;
	m_Camera.SetOrthogonalRect(rb.LeftTop.x, rb.LeftTop.y, rb.RightBottom.x, rb.RightBottom.y, -100.0f, 100.0f);

	if (!GetHandleTable()->Allocate(this, RootEntry.m_OwnerEntity, RootEntry.m_SelfHandle, index)) {
		AddLog(Error, "Failed to allocate root handle");
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}
	m_EntityMapper.SetComponentMapping(RootEntry);

	return true;
}

bool RectTransformComponent::Finalize() {
	return true;
}

//---------------------------------------------------------------------------------------

void RectTransformComponent::Step(const Core::MoveConfig & conf) {
	auto *EntityManager = GetManager()->GetWorld()->GetEntityManager();

	conf.CustomDraw.push_back(this);

	size_t LastInvalidEntry = 0;
	size_t InvalidEntryCount = 0;

	for (size_t i = 1; i < m_Array.Allocated(); ++i) {//ignore root entry
		auto &item = m_Array[i];

		if (!item.m_Flags.m_Map.m_Valid) {
			//mark and continue
			LastInvalidEntry = i;
			++InvalidEntryCount;
			continue;
		}

		Entity ParentEntity;
		if (EntityManager->GetParent(item.m_OwnerEntity, ParentEntity)) {
			auto *ParentEntry = GetEntry(ParentEntity);

			if (ParentEntry->m_Revision <= item.m_Revision && m_CurrentRevision > 1) {
				//nothing to do, nothing changed;
			} else {
				item.Recalculate(*ParentEntry);
				item.m_Revision = m_CurrentRevision;
			}
			//	item.m_GlobalScale = ParentEntry->m_GlobalScale * item.m_LocalScale;
		} else {
			item.m_Flags.m_Map.m_Valid = false;
			LastInvalidEntry = i;
			++InvalidEntryCount;
			continue;
			//mark and continue but set valid to false to avoid further processing
		}
	}

	if (InvalidEntryCount > 0) {
		AddLogf(Performance, "TransformComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
		TrivialReleaseElement(LastInvalidEntry);
	}

	++m_CurrentRevision;
	if (m_CurrentRevision < 1) {
		m_CurrentRevision = 1;
	}
}

//---------------------------------------------------------------------------------------

bool RectTransformComponent::Load(xml_node node, Entity Owner, Handle &hout) {
	size_t index;
	if (!m_Array.Allocate(index)) {
		AddLogf(Error, "Failed to allocate index!");
		return false;
	}
	auto &entry = m_Array[index];
	entry.m_Flags.ClearAll();
	if (!GetHandleTable()->Allocate(this, Owner, entry.m_SelfHandle, index)) {
		AddLog(Error, "Failed to allocate handle");
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}
	hout = entry.m_SelfHandle;
	entry.m_OwnerEntity = Owner;

	Entity Parent;
	if (!GetManager()->GetWorld()->GetEntityManager()->GetParent(Owner, Parent)) {
		AddLog(Error, "Failed to get Parent!");
		return false;
	}

	auto ParentEntry = GetEntry(Parent);
	if (!ParentEntry) {
		AddLog(Error, "Failed to get ParentEntry!");
		return false;
	}

	entry.m_Flags.m_Map.m_Dirty = true;

	x2c::Component::RectTransformComponent::RectTransformEntry_t rte;
	rte.ResetToDefault();
	if (!rte.Read(node)) {
		AddLog(Error, "Failed to read RectTransfromEntry!");
		return false;
	}

	entry.m_AlignMode = rte.m_AlignMode;

	entry.m_ScreenRect.LeftTop = rte.m_Position;
	entry.m_ScreenRect.RightBottom = rte.m_Position + rte.m_Size;

	entry.m_Margin = rte.m_Margin;
	entry.m_Position = rte.m_Position;
	entry.m_Size = rte.m_Size;

	int32_t rawz = static_cast<uint32_t>(rte.m_Z);
	rawz += static_cast<int32_t>(std::numeric_limits<uint16_t>::max()) / 2;
	entry.m_Z = static_cast<uint16_t>( rawz );

	if (rte.m_UniformMode != m_Flags.m_Map.m_UniformMode) {
		auto &root = GetRootEntry();
		if (m_Flags.m_Map.m_UniformMode) {
			//convert from pixel to uniform
			if (entry.m_AlignMode != AlignMode::Table) {
				auto half = root.m_Size / 2.0f;
				entry.m_Position = entry.m_Position / m_ScreenSize - half;
				entry.m_Size = entry.m_Size / m_ScreenSize * root.m_Size;
			}
			entry.m_Margin = entry.m_Margin / m_ScreenSize * root.m_Size;
		} else {
			//convert from uniform to pixel
			//NOT TESTED; MAY NOT WORK
			float Aspect = m_ScreenSize[0] / m_ScreenSize[1];
			auto half = Point(Aspect, 1.0f);
			if (entry.m_AlignMode != AlignMode::Table) {
				entry.m_Position = entry.m_Position * m_ScreenSize + half;
				entry.m_Size = entry.m_Size * m_ScreenSize;
			}
			entry.m_Margin *= m_ScreenSize;
		}
	} 

	entry.Recalculate(*ParentEntry);
	entry.m_Revision = m_CurrentRevision;

	entry.m_Flags.m_Map.m_Valid = true;
	m_EntityMapper.SetComponentMapping(entry);
	return true;
}

bool RectTransformComponent::LoadComponentConfiguration(pugi::xml_node node) {
	x2c::Component::RectTransformComponent::RectTransformEntry_t rts;
	rts.ResetToDefault();
	if (!rts.Read(node)) {
		AddLog(Error, "Failed to read settings!");
		return false;
	}

	m_Flags.m_Map.m_UniformMode = rts.m_UniformMode;
	
	return true;
}

//---------------------------------------------------------------------------------------

void RectTransformComponent::D2Draw(Graphic::cRenderDevice & dev) {
	if (!m_Shader) {
		if (!Graphic::GetShaderMgr()->GetSpecialShader("btDebugDraw.default", m_Shader)) {
			AddLogf(Error, "Failed to load btDebgDraw shader");
			return;
		}

	}

	if (!m_Shader)
		return;

	auto cs = dev.CurrentShader();

	dev.Bind(m_Shader);
	dev.Bind(&m_Camera);
	dev.SetModelMatrix(math::mat4());
	dev.BindNullMaterial();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	for (size_t i = 1; i < m_Array.Allocated(); ++i) {//ignore root entry
		auto &item = m_Array[i];
		if (!item.m_Flags.m_Map.m_Valid) {
			continue;
		}

		dev.CurrentShader()->SetBackColor(math::vec3(1, 1, 1));

		auto &r = item.m_ScreenRect;

		glBegin(GL_LINE_STRIP);
		glVertex3f(r.LeftTop.x, r.LeftTop.y, 0.0f);
		glVertex3f(r.LeftTop.x, r.RightBottom.y, 0.0f);
		glVertex3f(r.RightBottom.x, r.RightBottom.y, 0.0f);
		glVertex3f(r.RightBottom.x, r.LeftTop.y, 0.0f);
		glVertex3f(r.LeftTop.x, r.LeftTop.y, 0.0f);
		glEnd();
	}

	dev.Bind(cs);

	glPopAttrib();
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

void RectTransformComponentEntry::Recalculate(RectTransformComponentEntry &Parent) {
	const auto &parentmargin = Parent.m_Margin;
	const auto parentsize = Parent.m_ScreenRect.GetSize();

	bool doslice = true;

	switch (m_AlignMode) {
	case AlignMode::None:
		break;

	case AlignMode::Top:
		m_Position = parentmargin.LeftTopMargin();
		m_Size.x = parentsize.x - parentmargin.VerticalMargin();
		break;
	case AlignMode::Bottom:
		m_Size.x = parentsize.x - parentmargin.VerticalMargin();
		m_Position = Point(parentmargin.Left, parentsize.y - parentmargin.Top - m_Size.y);
		break;
	case AlignMode::Left:
		m_Position = parentmargin.LeftTopMargin();
		m_Size.y = parentsize.y - parentmargin.VerticalMargin();
		break;
	case AlignMode::Right:
		m_Position = Point(parentsize.x - parentmargin.Right - m_Size.x, parentmargin.Top);
		m_Size.y = parentsize.y - parentmargin.VerticalMargin();
		break;

	case AlignMode::LeftTop:
		m_Position = parentmargin.LeftTopMargin();
		break;
	case AlignMode::LeftBottom:
		m_Position = Point(parentmargin.Left, parentsize.y - parentmargin.Bottom - m_Size.y);
		break;
	case AlignMode::RightTop:
		m_Position = Point(parentsize.x - parentmargin.Right - m_Size.x, parentmargin.Top);
		break;
	case AlignMode::RightBottom:
		m_Position = parentsize - parentmargin.RightBottomMargin() - m_Size;
		break;

	case AlignMode::LeftMiddle: 
		m_Position = Point(parentmargin.Left, parentmargin.Top + (parentsize.y - parentmargin.VerticalMargin()) / 2.0f);
		break;
	case AlignMode::RightMiddle: 
		m_Position = Point(parentsize.x - parentmargin.Right - m_Size.x, parentmargin.Top + (parentsize.y - parentmargin.VerticalMargin()) / 2.0f);
		break;
	case AlignMode::MiddleTop: 
		m_Position = Point(parentmargin.Left + (parentsize.x - parentmargin.HorizontalMargin() - m_Size.x) / 2.0f, parentmargin.Top);
		break;
	case AlignMode::MiddleBottom: 
		m_Position = Point(parentmargin.Left + (parentsize.x - parentmargin.HorizontalMargin() - m_Size.x) / 2.0f, parentsize.y - parentmargin.Top - m_Size.y);
		break;

	case AlignMode::FillParent:
		m_Position = parentmargin.LeftTopMargin();
		m_Size = Point(parentsize.x - parentmargin.HorizontalMargin(), parentsize.y - parentmargin.VerticalMargin());
		break;

	case AlignMode::Center: {
		auto halfparent = parentsize / 2.0f;
		auto halfsize = m_Size / 2.0f;
		m_Position = halfparent - halfsize;
		break;
	}
	case AlignMode::Table: {
		auto cell = (parentsize - (parentmargin.TotalMargin())) / m_Size;
		auto cellpos = parentmargin.LeftTopMargin() + cell * m_Position;
		m_ScreenRect.SliceFromParent(Parent.m_ScreenRect, cellpos, cell);
		m_LocalMatrix = glm::translate(glm::mat4(), math::vec3(cellpos, 0));
		doslice = false;
		break;
	}

	default:
		LogInvalidEnum(m_AlignMode);
		break;
	}

	if (doslice) {
		m_ScreenRect.SliceFromParent(Parent.m_ScreenRect, m_Position, m_Size);
		m_LocalMatrix = glm::translate(glm::mat4(), math::vec3(m_Position, 0));
	}

	m_GlobalMatrix = Parent.m_GlobalMatrix * m_LocalMatrix;
}

} //namespace Component 
} //namespace GUI 
} //namespace MoonGlare 