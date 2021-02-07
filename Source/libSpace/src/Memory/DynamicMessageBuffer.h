
#include "../Configuration.h"
#include "LockPolicy.h"

namespace Space {
namespace Memory {


template<size_t LENGTH, class HEADER, class LOCKPOLICY = DefaultLockPolicy, bool ENABLEFRAMING = false>
struct DynamicMessageBuffer {
	using Header = HEADER;
	using LockPolicy = LOCKPOLICY;
	enum {
		Size = LENGTH,
		FramingEnabled = ENABLEFRAMING ? 1 : 0,

		HeaderMagic = 0x46696A59,
		FooterMagic = 0x26A514F3,
	};
	static_assert(std::is_pod<Header>::value, "DynamicMessageBuffer header must be a POD type");

	struct FrameHeader {
		uint32_t m_Magic;
		uint32_t m_FrameSize;
	};
	struct FrameFooter {
		uint32_t m_Magic;
	};

	DynamicMessageBuffer() : m_UsedSize(0), m_PullLocation(0), m_CurrentHEader(nullptr) { Clear(); }

	void CloneFrom(const DynamicMessageBuffer& other) {
		auto lock = m_Lock.Lock();
		auto lock2 = other.m_Lock.Lock();
		m_UsedSize = other.m_UsedSize;
		m_PullLocation = other.m_PullLocation;
		memcpy(m_Buffer, other.m_Buffer, Size);
	}

	void Fill(char value) {
		auto lock = m_Lock.Lock();
		memset(m_Buffer, value, Size);
	}
	void Clear() { m_PullLocation = m_UsedSize = HeaderSize(); }

	const char* GetBuffer() const { return m_Buffer; }
	size_t UsedSize() const { return m_UsedSize; }
	size_t HeaderSize() const { return sizeof(Header); }
	size_t PayLoadSize() const { return UsedSize() - HeaderSize(); }

	//template<class T> bool Push(T & t) {
	//	Check<T>();
	//	auto lock = m_Lock.Lock();
	//	if (m_UsedSize + sizeof(T) > Size)
	//		return false;
	//	memcpy(m_Buffer + m_UsedSize, &t, sizeof(T));
	//	m_UsedSize += sizeof(T);
	//	return true;
	//}
	//template<class T> bool Push(T * t) {
	//	Check<T>();
	//	auto lock = m_Lock.Lock();
	//	if (m_UsedSize + sizeof(T) > Size)
	//		return false;
	//	memcpy(m_Buffer + m_UsedSize, t, sizeof(T));
	//	m_UsedSize += sizeof(T);
	//	return true;
	//}
	bool PushString(const char *t) {
		size_t len = (t ? strlen(t) : 0) + 1;
		auto lock = m_Lock.Lock();
		if (m_UsedSize + len > Size)
			return false;
		if (t)
			memcpy(m_Buffer + m_UsedSize, t, len);
		else
			m_Buffer[m_UsedSize] = 0;
		m_UsedSize += len;
		return true;
	}
	bool PushString(const std::string &t) {
		size_t len = t.length() + 1;
		auto lock = m_Lock.Lock();
		if (m_UsedSize + len > Size)
			return false;
		if (!t.empty())
			memcpy(m_Buffer + m_UsedSize, t.c_str(), len);
		else
			m_Buffer[m_UsedSize] = 0;
		m_UsedSize += len;
		return true;
	}
	template<class T> T* Alloc() {
		Check<T>();
		auto lock = m_Lock.Lock();
		if (m_UsedSize + sizeof(T) > Size)
			return nullptr;
		T *t = reinterpret_cast<T*>(m_Buffer + m_UsedSize);
		m_UsedSize += sizeof(T);
		return t;
	}
	template<class T> T* AllocAndZero() {
		auto *t = Alloc<T>();
		if (!t) return t;
		memset(t, 0, sizeof(T));
		return t;
	}

	Header* GetHeader() {
		return reinterpret_cast<Header*>(m_Buffer);
	}

	template<class T> void Pull() {
		Check<T>();
		auto lock = m_Lock.Lock();
		m_PullLocation += sizeof(T);
	}
	const char* PullString() {
		const char *c = m_Buffer + m_PullLocation;
		auto lock = m_Lock.Lock();
		for (; m_Buffer[m_PullLocation]; ++m_PullLocation);
		if (!m_Buffer[m_PullLocation])
			++m_PullLocation;
		return c;
	}
	template<class T> T* Get() {
		Check<T>();
		return reinterpret_cast<T*>(m_Buffer + m_PullLocation);
	}
	template<class T> T* GetAndPull() {
		Check<T>();
		auto lock = m_Lock.Lock();
		T* t = Get<T>();
		Pull<T>();
		return t;
	}
private:
	mutable LockPolicy m_Lock;
	size_t m_UsedSize;
	size_t m_PullLocation;
	FrameHeader *m_CurrentHEader;
	char m_Buffer[Size];

	template<class T>
	void Check() {
		static_assert(std::is_pod<T>::value, "DynamicMessageBuffer accepts only POD types");
	}
};

}
}
