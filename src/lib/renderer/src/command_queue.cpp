#include "command_queue.hpp"

namespace MoonGlare::Renderer {

#if 0

void CommandQueue::SortBegin(int left, int right) {
    if (left >= right)
  {      return;}

    int pivot = (left + right) / 2;
    auto pivotkey = m_SortKeys[pivot];
    auto pivotptr = m_CommandArguments[pivot];
    auto pivotfunc = m_CommandFunctions[pivot];

    auto Comp = [this, pivotkey, pivotptr](int a, int) -> bool {
        auto ak = m_SortKeys[a];
        auto bk = pivotkey; // m_SortKeys[b];
        if (ak.m_UIntValue == bk.m_UIntValue) {
            return m_CommandArguments[a] < pivotptr;
        }
        return ak.m_UIntValue < bk.m_UIntValue;
    };
    auto Comp2 = [this, pivotkey, pivotptr](int, int b) -> bool {
        auto ak = pivotkey;
        auto bk = m_SortKeys[b]; // m_SortKeys[b];
        if (ak.m_UIntValue == bk.m_UIntValue) {
            return pivotptr < m_CommandArguments[b];
        }
        return ak.m_UIntValue < bk.m_UIntValue;
    };
    auto swap = [this](int a, int b) {
        std::swap(m_CommandFunctions[a], m_CommandFunctions[b]);
        std::swap(m_CommandArguments[a], m_CommandArguments[b]);
        std::swap(m_SortKeys[a], m_SortKeys[b]);
    };

    swap(pivot, right);

    int i, j;
    for (j = i = left; i < right; i++)
        if (Comp(i, 0)) {
            swap(i, j);
            j++;
        }

    swap(right, j);

    m_SortKeys[j] = pivotkey;
    m_CommandArguments[j] = pivotptr;
    m_CommandFunctions[j] = pivotfunc;

    if (left < j - 1)
        SortBegin(left, j - 1);
    if (j + 1 < right)
        SortBegin(j + 1, right);
}

#endif

} // namespace MoonGlare::Renderer