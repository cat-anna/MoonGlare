find_path(LUA_INCLUDE_DIR lua.hpp)
find_library(LUA_LIBRARY lua51)

message("Lua include: " ${LUA_INCLUDE_DIR})
message("Lua library: " ${LUA_LIBRARY})

include_directories(${LUA_INCLUDE_DIR})
link_libraries(${LUA_LIBRARY})
