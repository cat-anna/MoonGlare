#pragma once

#include <lua.hpp>

namespace MoonGlare::Scripts {

inline void StackDump(lua_State *L) {
    int i;
    int top = lua_gettop(L);
    for (i = 1; i <= top; i++) {  /* repeat for each level */
        int t = lua_type(L, i);
        printf("%s", lua_typename(L, t));
        printf("  ");  /* put a separator */
    }
    printf("\n");  /* end the listing */
}

}
