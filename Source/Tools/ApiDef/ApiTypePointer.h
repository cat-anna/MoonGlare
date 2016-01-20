#ifndef id737AB3D7_7C63_4E63_9F17286335F19748
#define id737AB3D7_7C63_4E63_9F17286335F19748

#include "ApiBaseType.h"

class TypePointer : protected ApiType {
protected:
    mutable const ApiBaseType *m_Ptr;
public:
	const std::string &GetTypeName() const;
	void SetType(const std::string &Type);
	
	const ApiBaseType *GetType(const ApiBaseType *Owner) const;

	TypePointer();
	TypePointer(const std::string &Type);
		
	TypePointer& operator = (const std::string &type);
	
	using ApiType::SaveToXML;
	using ApiType::LoadFromXML;
	using ApiType::ToString;
	using ApiType::CurrentPath;
};

#endif // header
