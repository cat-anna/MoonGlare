#include <cstdio>
#include "RTTI.h"

namespace Space {
namespace RTTI {

//----------------------------GabiRTTIObject-----------------------------------------------

const TypeInfo RTTIObject::m_TypeInfo("Object", typeid(TypeInfo).name(), NULL, sizeof(RTTIObject), nullptr, false, false);

RTTIObject::RTTIObject() {
}

RTTIObject::~RTTIObject() {
}

bool RTTIObject::IsInstanceOf (const TypeInfo* TypeInfo) {
    return IsBaseClass(TypeInfo, this->GetDynamicTypeInfo());
}

bool RTTIObject::IsClassInstance(const RTTIObject* Obj, const TypeInfo* TypeInfo) {
	return IsBaseClass(TypeInfo, Obj->GetDynamicTypeInfo());
}

bool RTTIObject::IsBaseClass(const TypeInfo* BaseType, const TypeInfo* InstanceType) {
	if(!BaseType) return false;
	while (InstanceType) {
		if (BaseType == InstanceType) return true;
		InstanceType = InstanceType->m_Base;
	}
	return false;
}

//----------------------------GabiTracker-----------------------------------------------

Tracker::TypeList* Tracker::_List = 0;

void Tracker::InsertClass(const TypeInfo * Class){
	ForceList();
	_List->push_back(Class);
}

bool PComp(const TypeInfo *a, const TypeInfo *b) {
#ifdef _MSC_VER
	return stricmp(a->GetName(), b->GetName()) < 0;
#else
	return strcasecmp(a->GetFullName(), b->GetFullName()) < 0;
#endif
}

void Tracker::SortList() {
	_List->sort(PComp);
}

void Tracker::DumpClasees(std::ostream &out){
	ForceList();
	int n = 0;
	char buf[1024];

	sprintf_s(buf, TypeInfo::PrintLineFormat, "No. ", "Class", "Used", "Size;%16", "Dynamic", "Abstract", "Singleton", "POD");
	out << buf;

	unsigned used = 0;
	unsigned dynamic = 0;
	unsigned abstract = 0;
	unsigned singleton = 0;
	unsigned pod = 0;

	for (iterator it = begin(), jt = end(); it != jt; ++it){
		const TypeInfo *i = *it;
		++n;
		const char* usedstr = (i->InstancesCount + i->DeletedCount ? "Yes" : "");
		if (i->InstancesCount + i->DeletedCount)
			++used;
		
		char nbuf[32];
		sprintf_s(nbuf, "%-3d ", n);
		out << nbuf << *i << "\n";

		if (!i->m_Flags.m_Static)
			++dynamic;
		if (i->m_Flags.m_Abstract)
			++abstract;
		if (i->m_Flags.m_Singleton)
			++singleton;
		if (i->m_Flags.m_POD)
			++pod;
	}
	out << "--------------------------------------------\n";
	sprintf_s(buf, "%3s  %70s %4d %5s    %-4d    %-5d    %-6d %-3d\n", "", "Summary:", used, "", dynamic, abstract, singleton, pod);
	out << buf << "\n";
}

void Tracker::DumpInstances(std::ostream &out) {
	ForceList();
	int index = 1;
	for(iterator it = begin(), jt = end(); it != jt; ++it){
		const TypeInfo *i = *it;
		char buf[512];
#ifdef SPACERTTI_TRACK_INSTANCES
		if(!i->InstancesCount && !i->DeletedCount)
			continue;
		sprintf_s(buf, "%3d. Instances:%5d Deleted:%5d -> Class:'%s'\n", index, i->InstancesCount, i->DeletedCount, i->GetName());
#else
		sprintf_s(buf, "%3d. Class:'%s'\n", index, i->m_Name);
#endif
		++index;
		out << buf;
	}
}

} //namespace RTTI
} //namespace Space
