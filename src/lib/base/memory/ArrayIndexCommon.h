#pragma once

#include <boost/tti/has_member_function.hpp>
#include <type_traits>

namespace MoonGlare::Memory
{
namespace detail
{
BOOST_TTI_HAS_MEMBER_FUNCTION(ReleaseElement)
BOOST_TTI_HAS_MEMBER_FUNCTION(InitElemenent)
BOOST_TTI_HAS_MEMBER_FUNCTION(SwapValues)
BOOST_TTI_HAS_MEMBER_FUNCTION(GetElementName)
BOOST_TTI_HAS_MEMBER_FUNCTION(ClearArrays)

template<typename T, typename IsEnumType> struct ArrayIndexIndexType { };
template<typename T> struct ArrayIndexIndexType<T, std::true_type> { using IntType = std::underlying_type_t<T>; };
template<typename T> struct ArrayIndexIndexType<T, std::false_type> { using IntType = T; };

} // namespace detail

}