#pragma once

#include <list>
#include <functional>
#include <mutex>

namespace MoonGlare::Memory {

template<class CALLABLE>
struct BasicActionQueue {
	typedef CALLABLE Func_t ;
	typedef std::list<Func_t> Container_t;

	template<class T>
	void Add(T&& t) {
		std::lock_guard < std::mutex > lock(m_mutex);
		m_queue.emplace_back(std::forward<T>(t));
	}

	template<class ... Args>
	bool DispatchSingleAction(Args&&... args) {
		if (m_queue.empty()) return false;
		Func_t f;
		{
			std::lock_guard < std::mutex > lock(m_mutex);
			std::swap(f, m_queue.front());
			m_queue.pop_front();
		}
		f(std::forward<Args>(args)...);
		return true;
	}

	template<class ... Args>
	void DispatchPendingActions(Args&&... args) {
		while (DispatchSingleAction(std::forward<Args>(args)...));
	}

    template<class ... Args>
    void AsyncDispatchAll(Args&&... args) {
        if (m_queue.empty())
            return;
        Container_t queue;
        {
            std::lock_guard < std::mutex > lock(m_mutex);
            queue.swap(m_queue);
        }
        for (auto &f : queue) {
            f(std::forward<Args>(args)...);
        }
    }

	size_t Count() const { return m_queue.size(); }
private:
	std::mutex m_mutex;
	Container_t m_queue;
};

using ActionQueue = BasicActionQueue < std::function<void()> > ;

} //namespace Space
