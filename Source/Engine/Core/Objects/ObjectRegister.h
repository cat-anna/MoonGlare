/*
 * cObjectList.h
 *
 *  Created on: 09-12-2013
 *      Author: Paweu
 */

#ifndef COBJECTLIST_H_
#define COBJECTLIST_H_
namespace Core {
namespace Objects {

class ObjectList : public std::list < Object* > { 
public:
	using ListBase =  std::list < Object* >;
	struct ListTypeIterator {
		ListTypeIterator(ListBase::const_iterator begin, ListBase::const_iterator end):
			m_begin(begin), m_end(end) { };
		~ListTypeIterator() { }
		ListTypeIterator(const ListTypeIterator& i) : m_begin(i.m_begin), m_end(i.m_end), m_index(i.m_index) { }

		bool Ended() { return m_begin == m_end; }
		bool Next() { ++m_begin; ++m_index; return Ended(); }
		int Index() { return m_index; }
		Object* Get() { if (Ended()) return 0; return *m_begin; }
	protected:
		ListBase::const_iterator m_begin, m_end;
		int m_index = 0;
	};

	ListTypeIterator GetIterator() const { return ListTypeIterator(begin(), end()); }

	size_t isize() const { return size(); }
	ObjectList() { }
	~ObjectList() { }
};

using UniqueObjectRegister = std::unique_ptr<ObjectRegister>;

class ObjectRegister : public cRootClass {
	GABI_DECLARE_CLASS_NOCREATOR(ObjectRegister, cRootClass)
public:
	using iterator = ObjectList::iterator;
	using const_iterator = ObjectList::const_iterator;

	ObjectRegister();
	~ObjectRegister();

	iterator begin() { return m_List.begin(); }
	iterator end() { return m_List.end(); }
	const_iterator begin() const { return m_List.begin(); }
	const_iterator end() const { return m_List.end(); }
	size_t count() const { return m_List.size(); }

	Object* GetFirstObjectByName(const string& Name) {
		auto &list = m_NameMap[Name];
		if (list.empty()) return 0;
		return list.front();
	}

	const ObjectList& GetObjectsByName(const string& Name) {
		return m_NameMap[Name];
	}

	const ObjectList& GetObjectsByType(const string& Type) {
		return m_TypeMap[Type];
	}

	void Remove(Object* object) { 
		if (!object) {
			AddLog(Error, "Attempt to remove null object from register!");
			return;
		}
		m_List.remove(object); 
		m_NameMap[object->GetName()].remove(object);
		m_TypeMap[object->GetPatternName()].remove(object);
	}
	void Add(Object* object) { 
		if (!object) {
			AddLog(Error, "Attempt to add null object to register!");
			return;
		}
		m_List.push_back(object); 
		m_NameMap[object->GetName()].push_back(object);
		m_TypeMap[object->GetPatternName()].push_back(object);
	}

	void Clear();
	static void RegisterScriptApi(ApiInitializer &api);

	bool LoadObjects(const xml_node SrcNode, GameScene *OwnerScene);
protected:
	std::unordered_map<string, ObjectList> m_NameMap;
	std::unordered_map<string, ObjectList> m_TypeMap;
	ObjectList m_List;
};

} // namespace Objects
} // namespace Core
#endif // COBJECTLIST_H_ 
