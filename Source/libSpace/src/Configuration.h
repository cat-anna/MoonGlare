
#include <type_traits>
#include <typeinfo>

#include <atomic>

#include <functional>

#include <vector>
#include <list>
#include <string>

#include <iostream>

#include <cstdlib>
#include <cstring>

#define LIBSPACE_ASSERT(cond)  if(!(cond)) throw std::runtime_error("'" #cond "'" )
//#define LIBSPACE_ASSERT(cond) if(!(cond)) __debugbreak()
//do { } while(false)
//if(!(cond)) __debugbreak()
#define LIBSPACE_BOUNDARY_ASSERT(cond) LIBSPACE_ASSERT(cond)
//do { } while(false)

#define LIBSPACE_ASSERT_NOEXCEPT 
