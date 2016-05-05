/*
 * cScriptEvents.h
 *
 *  Created on: 06-12-2013
 *      Author: Paweu
 */

#ifndef CSCRIPTEVENTS_H_
#define CSCRIPTEVENTS_H_
namespace Core {
	namespace Scripts {

#define SCRIPT_EVENT_ADD(...)					__VA_ARGS__
#define SCRIPT_EVENT_REMOVE(...)				__VA_ARGS__
#define SCRIPT_EVENT_VECTOR_MAKE_STRING(A)		#A

#define SCRIPT_EVENT_VECTOR_ADD_IMPL(r, data, elem)			Add(SCRIPT_EVENT_VECTOR_MAKE_STRING(elem), & data :: elem);
#define SCRIPT_EVENT_VECTOR_REMOVE_IMPL(r, data, elem)		Remove(SCRIPT_EVENT_VECTOR_MAKE_STRING(elem));
#define SCRIPT_EVENT_DECLARE(r, data, elem)					string elem;

#define DECLARE_SCRIPT_EVENT_VECTOR(CLASS, BASE, ADD, REMOVE)								\
	class CLASS : public BASE {																\
		SPACERTTI_DECLARE_STATIC_CLASS(CLASS, BASE);												\
	protected:																				\
		class EventVector : public BASE::EventVector {										\
		public:																				\
			using BASE::EventVector::Add;													\
			EventVector() : BASE::EventVector() {											\
				BOOST_PP_SEQ_FOR_EACH(SCRIPT_EVENT_VECTOR_ADD_IMPL, CLASS, ADD);			\
				BOOST_PP_SEQ_FOR_EACH(SCRIPT_EVENT_VECTOR_REMOVE_IMPL, CLASS, REMOVE);		\
				}																			\
			};																				\
		static EventVector _Events;															\
		virtual EventVectorRoot &GetEventList() const override { return CLASS::_Events; };  \
	public:																					\
		CLASS(): BaseClass() { }															\
		BOOST_PP_SEQ_FOR_EACH(SCRIPT_EVENT_DECLARE, 0, ADD);								\
	}																						\

#define IMPLEMENT_SCRIPT_EVENT_VECTOR(CLASS)												\
	SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(CLASS)													\
	CLASS::EventVector CLASS::_Events														\

#define DECLARE_SCRIPT_HANDLERS_ROOT(CLASS)													\
	protected:																				\
		void SetScriptHandlers(CLASS *ptr) { m_ScriptHandlers.Set(ptr); }					\
		::Core::Scripts::ScriptEventBaseHolder<CLASS> m_ScriptHandlers;						\
	public:																					\
		void SetEventFunction(const string& Event, const string& Function) { 				\
			m_ScriptHandlers->Set(Event, Function); 										\
		}																					\
		DECLARE_SCRIPT_HANDLERS(CLASS)														\

#define DECLARE_SCRIPT_HANDLERS(CLASS)														\
	public:																					\
		using ScriptEventClass = CLASS;														\
	protected:																				\
		const CLASS* GetScriptEvents() const { return m_ScriptHandlers.get<CLASS>(); }		\
		CLASS* GetScriptEvents() { return m_ScriptHandlers.get<CLASS>(); }					\

class iScriptEvents : public cRootClass {
	SPACERTTI_DECLARE_ABSTRACT_CLASS(iScriptEvents, cRootClass)
protected:
	class EventVector : protected std::unordered_map<string, string iScriptEvents::*> {
	protected:
		template<class OWNER>
		EventVector &Add(const char* Name, string OWNER::*eptr) {
			BaseClass::insert(std::make_pair(Name, static_cast<string iScriptEvents::*>(eptr)));
			return *this;
		}
		EventVector &Remove(const char* Name);
	public:
		typedef std::unordered_map<string, string iScriptEvents::*> BaseClass;
		using BaseClass::find;
		using BaseClass::begin;
		using BaseClass::end;
		using BaseClass::const_iterator;
		using BaseClass::iterator;
	};

	typedef iScriptEvents ScriptEventsRoot;
	typedef EventVector EventVectorRoot;

	/** Get event table of current script events instance. */
	virtual EventVectorRoot &GetEventList() const = 0;
	virtual void InternalInfo(std::ostringstream &buff) const;
public:
	/** Set event function name */
	void Set(const string& Event, const string& Function);
	/** Save event function bindings to xml. Node 'Events' is added automaticaly. */
	bool SaveToXML(xml_node Parent) const;
	/** Load script function bindings from xml. Loads from subnode 'Events'. */
	bool LoadFromXML(xml_node Parent);
	ThisClass* operator->() { return this; }
	/** Assign bindings from other instance */
	void Assign(const ThisClass& Source);
};

template<class T>
class ScriptEventBaseHolder {
	std::unique_ptr<T> m_ptr;
public:
	typedef ScriptEventBaseHolder<T> ThisClass;

	ScriptEventBaseHolder(T *ptr = new T()) : m_ptr(ptr) { }
	~ScriptEventBaseHolder() { Set(nullptr); }

	template<class REQUEST = T>
	const REQUEST* get() const { 
		THROW_ASSERT(dynamic_cast<REQUEST*>(m_ptr.get()), "Invalid script event class!");
		return (REQUEST*)m_ptr.get(); 
	}

	template<class REQUEST = T>
	REQUEST* get() { 
		THROW_ASSERT(dynamic_cast<REQUEST*>(m_ptr.get()), "Invalid script event class!");
		return (REQUEST*)m_ptr.get(); 
	}

	/** Set new script event instance. */
	void Set(T *ptr) { m_ptr.reset(ptr); }
	/** Set new script event instance. */
	void Reset(T *ptr = nullptr) { Set(ptr); }

	const T* operator->() const { return m_ptr.get(); };
	T* operator->() { return m_ptr.get(); };
	operator bool() const { return static_cast<bool>(m_ptr); }
	ThisClass& operator=(T* ptr) { Set(ptr); }
};

} //namespace Scripts
} //namespace Core

#endif // CSCRIPTEVENTS_H_ 
