#pragma once

#if 0

#include "../Configuration.Renderer.h"
#include "CommandQueue.h"

namespace MoonGlare::Renderer::Commands {

template<typename LAYERENUM>
class CommandQueueLayers {
public:
	using LayerEnum = LAYERENUM;
	using Queue = CommandQueue;

	static constexpr uint32_t LayerCount = static_cast<uint32_t>(LayerEnum::MaxValue);

	void Sort() {
		for (auto &it : m_Layers)
			it.Sort();
	}
	void Execute() {
		for (auto &it : m_Layers)
			it.Execute();
	}
	void ClearAllocation() {
		for (auto &it : m_Layers)
			it.ClearAllocation();
	}
	void MemZero() {
		for (auto &it : m_Layers)
			it.MemZero();
	}
	void Clear() {
		for (auto &it : m_Layers)
			it.Clear();
	}

    template<LayerEnum ... Layers>
    void Execute() {
        int v[] = { (Get<Layers>().Execute(), 1)... };
    }


	template<LayerEnum layer>
	Queue& Get() { return m_Layers[static_cast<uint32_t>(layer)]; }
	Queue& operator[](LayerEnum layer) { return m_Layers[static_cast<size_t>(layer)]; }
private:
	std::array<Queue, LayerCount> m_Layers;
};

} //namespace MoonGlare::Renderer::Commands
#endif