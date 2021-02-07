#pragma once

template<size_t B, class OWNER>
std::string PrintHandle(DoubleHandle32<B> h, OWNER* owner) {
	std::stringstream out;
	out << "{";
	out << " Index:" << h.GetIndex();
	out << " Generation:" << (uint64_t)h.GetGeneration();
	if (owner)
		out << " RawIndex:" << owner->GetRawIndex(h);
	out << " }";
	return out.str();
}

