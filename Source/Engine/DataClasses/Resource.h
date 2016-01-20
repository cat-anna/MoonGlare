/*
 * cDataManager.h
 *
 *  Created on: 19-11-2013
 *      Author: Paweu
 */

#ifndef CRESOURCEMANAGER_H_
#define CRESOURCEMANAGER_H_
namespace DataClasses {

struct NameClassPair {
	string Name, Class;

	bool LoadMeta(const xml_node node) {
		const char *c, *n;
		c = node.attribute(xmlAttr_Class).as_string(0);
		if (!c) return false;
		n = node.attribute(xmlAttr_Name).as_string(0);
		if (!n) return false;
		Name = n;
		Class = c;
		return true;
	}

	bool SaveMeta(xml_node node) const {
		LOG_NOT_IMPLEMENTED;
		return false;
	}
};

struct NameClassSourceTriplet {
	string Name, Class;
	xml_node Src;

	bool LoadMeta(const xml_node node) {
		const char *c, *n;
		c = node.attribute(xmlAttr_Class).as_string(0);
		if (!c) return false;
		n = node.attribute(xmlAttr_Name).as_string(0);
		if (!n) return false;
		Name = n;
		Class = c;
		Src = node;
		return true;
	}

	bool SaveMeta(xml_node node) const {
		LOG_NOT_IMPLEMENTED;
		return false;
	}
};

class NameClassList : public std::list<NameClassPair> {
public:
	bool LoadMeta(const xml_node parent, const char *ChildName) {
		bool succ = false;
		XML::ForEachChild(parent, ChildName, [this, &succ](xml_node node) ->int {
			push_back(NameClassPair());
			auto &item = back();
			if (item.LoadMeta(node))
				succ = true;
			return 0;
		});
		return succ;
	}
};

class NameClassSourceList : public std::list<NameClassSourceTriplet> {
public:
	bool LoadMeta(const xml_node parent, const char *ChildName) {
		bool succ = false;
		XML::ForEachChild(parent, ChildName, [this, &succ](xml_node node) ->int {
			push_back(NameClassSourceTriplet());
			auto &item = back();
			if (item.LoadMeta(node))
				succ = true;
			return 0;
		});
		return succ;
	}
};

class ResourcePointerBase {
public:
	ResourcePointerBase(DataModule *Owner = nullptr) : m_Owner(Owner), m_Flags(0) { }
	ResourcePointerBase(DataModule *Owner, const NameClassPair& info) : m_Owner(Owner), m_Info(info), m_Flags(0) { }
	ResourcePointerBase(const NameClassPair& info) : m_Owner(nullptr), m_Info(info), m_Flags(0) { }

	const string& GetName() const { return m_Info.Name; }
	const string& GetClass() const { return m_Info.Class; }
	const DataModule* GetOwner() const { return m_Owner; }
	bool LoadMeta(const xml_node Node) { return m_Info.LoadMeta(Node); }
	bool SaveMeta(xml_node Node) const { return m_Info.SaveMeta(Node); }
	DefineFlag(m_Flags, 0x01, Valid);
protected:
	DataModule *m_Owner;
	NameClassPair m_Info;
	unsigned m_Flags;
};

template<class RESTYPE>
class ResourcePointer : public ResourcePointerBase {
public:
	typedef RESTYPE Res_t;

	DataModule* GetOwner() const { return m_Owner; }
	bool IsLoaded() const { return m_Ptr != 0; }
	template<class ...Args>
	RESTYPE* GetResource(Args ...args) const {
		if (IsLoaded()) return m_Ptr;
		m_Ptr = m_Owner->LoadResource(this, std::forward<Args>(args)...);
		return m_Ptr;
	}

	void ReleaseResource() { LOG_NOT_IMPLEMENTED; }
	unsigned UseCount() const { return 0; }

	ResourcePointer(DataModule *Owner) : ResourcePointerBase(Owner), m_Ptr(0){}
	ResourcePointer(DataModule *Owner, const NameClassPair& info) : ResourcePointerBase(Owner, info), m_Ptr(0) { }
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
class SmartResourceOld : public ResourcePointerBase {
public:
	typedef SMART Smart_t;

	bool IsLoaded() const { return static_cast<bool>(m_Ptr); }
	template<class ...Args>
	Smart_t GetResource(Args ...args) const {
		if (IsLoaded()) return m_Ptr;
		m_Ptr = m_Owner->LoadResource(this, std::forward<Args>(args)...);
		return m_Ptr;
	}
	unsigned UseCount() const { return m_Ptr.use_count(); }
	void ReleaseResource() { 		
		if (m_Ptr) {
			m_Ptr->Finalize();
			m_Ptr.reset();
		} 
	}

	SmartResourceOld(DataModule *Owner) : ResourcePointerBase(Owner), m_Ptr(0) { }
	SmartResourceOld(DataModule *Owner, const NameClassPair& info) : ResourcePointerBase(Owner, info), m_Ptr(0) { }
	~SmartResourceOld() { ReleaseResource(); }
private:
	mutable Smart_t m_Ptr;
};

template<class SMART>
struct SmartResource : public ResourcePointerBase {
	typedef SMART Smart_t;

	bool IsLoaded() const { return static_cast<bool>(m_Ptr); }
	Smart_t Get() const { return m_Ptr; }
	template<class T>void Set(T&& t, const string& Class) { m_Ptr = t; m_Info.Class = Class; SetValid(true); }
	void ReleaseResource() { m_Ptr.reset();	}
	unsigned UseCount() const { return m_Ptr.use_count(); }

	SmartResource() : ResourcePointerBase(), m_Ptr() { }
	SmartResource(const NameClassPair& info) : ResourcePointerBase(info), m_Ptr() { }
	~SmartResource() { ReleaseResource(); }
private:
	Smart_t m_Ptr;
};

template<class RESTYPE>
class ResourceDefinition : public ResourcePointerBase {
public:
	typedef RESTYPE Ptr_t;

	DataModule* GetOwner() const { return m_Owner; }
	bool IsLoaded() const { return m_Ptr != 0; }
	template<class ...Args>
	Ptr_t GetResource(Args ...args) const {
		return m_Owner->LoadResource(this, std::forward<Args>(args)...);
	}

	void ReleaseResource() { }

	ResourceDefinition(DataModule *Owner) : ResourcePointerBase(Owner) { }
	ResourceDefinition(DataModule *Owner, const NameClassPair& info) : ResourcePointerBase(Owner) {
		m_Class = info.Class;
		m_Name = info.Name;
	}
	~ResourceDefinition() { }
	ResourceDefinition(const ResourceDefinition&) = delete;
	ResourceDefinition& operator=(const ResourceDefinition&) = delete;
};

} // namespace DataClasses
#endif // CDATAMANAGER_H_ 
