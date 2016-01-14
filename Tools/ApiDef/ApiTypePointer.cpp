#include "ApiBaseType.h"

using namespace std;
using namespace pugi;

TypePointer::TypePointer(): ApiType("?", 0, stTypePointer), m_Ptr(0) {
}

TypePointer::TypePointer(const std::string &Type): ApiType(Type, 0, stTypePointer), m_Ptr(0) {
}

void TypePointer::SetType(const std::string &Type) { 
	m_Name = Type; 
	m_Ptr = 0;
}

TypePointer& TypePointer::operator = (const std::string &type) { 
	SetType(type);
	return *this;
}

const std::string &TypePointer::GetTypeName() const { 
	return m_Name; 
}

const ApiBaseType *TypePointer::GetType(const ApiBaseType *Owner) const {
	if(m_Ptr) return m_Ptr;
	if(!Owner) return 0;
	const ApiHeader *hdr = Owner->GetRootHeader();
	m_Ptr = hdr->FindType(GetTypeName());
	if(m_Ptr)
		return m_Ptr;
	return m_Ptr = hdr->GetOwner()->FindType(GetTypeName());
}
