/*
 * cDataManager.h
 *
 *  Created on: 19-11-2013
 *      Author: Paweu
 */

#ifndef CRESOURCEMANAGER_H_
#define CRESOURCEMANAGER_H_

namespace MoonGlare {
namespace DataClasses {

class ResourcePointerBase {
public:
	ResourcePointerBase() : m_Flags(0) { }

	//const string& GetName() const { return m_Info.Name; }
	//const string& GetClass() const { return m_Info.Class; }
	//bool LoadMeta(const xml_node Node) { return m_Info.LoadMeta(Node); }
	//bool SaveMeta(xml_node Node) const { return m_Info.SaveMeta(Node); }
	DefineFlag(m_Flags, 0x01, Valid);
protected:
	unsigned m_Flags;
};

template<class RESTYPE>
class ResourcePointer : public ResourcePointerBase {
public:
	typedef RESTYPE Res_t;

	bool IsLoaded() const { return m_Ptr != 0; }
	template<class T, class ...Args>
	RESTYPE* GetResource(T *Loader, Args ...args) const {
		if (IsLoaded()) return m_Ptr;
		m_Ptr = Loader->LoadResource(this, std::forward<Args>(args)...);
		return m_Ptr;
	}

	void ReleaseResource() { LOG_NOT_IMPLEMENTED(); }
	unsigned UseCount() const { return 0; }

	ResourcePointer() : ResourcePointerBase(), m_Ptr(0){}
	~ResourcePointer() {
		if (m_Ptr) {
			m_Ptr->Finalize();
			delete m_Ptr;
			m_Ptr = 0;
		}
	}
	ResourcePointer(const ResourcePointer&) = delete;
	ResourcePointer(const ResourcePointer&&) = delete;
	ResourcePointer& operator=(const ResourcePointer&) = delete;
private:
	mutable RESTYPE *m_Ptr;
};

template<class SMART>
struct SmartResource : public ResourcePointerBase {
	typedef SMART Smart_t;

	bool IsLoaded() const { return static_cast<bool>(m_Ptr); }
	Smart_t Get() const { return m_Ptr; }
	template<class T>void Set(T&& t) { m_Ptr = t; SetValid(true); }
	void ReleaseResource() { m_Ptr.reset();	}
	unsigned UseCount() const { return m_Ptr.use_count(); }

	SmartResource() : ResourcePointerBase(), m_Ptr() { }
	~SmartResource() { ReleaseResource(); }
private:
	Smart_t m_Ptr;
};

} // namespace DataClasses
} //namespace MoonGlare 

#endif // CDATAMANAGER_H_ 
