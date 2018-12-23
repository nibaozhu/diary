#include <lua.hpp>
#include <iostream>

int main(int argc, char **argv)
{
	lua_State *L = luaL_newstate();
	std::cout << L << std::endl;

	const char *filename = "hello.lua";
	int r = luaL_dofile(L, filename);
	std::cout << r << std::endl;

	const char *name = "str";
	lua_getglobal (L, name);
	std::string str = lua_tostring(L, -1);
	std::cout << str.c_str() << std::endl;

	lua_getglobal (L, "tbl");
	lua_getfield (L, -1, "name");
	str = lua_tostring(L, -1);
	std::cout << str.c_str() << std::endl;

	lua_getglobal (L, "tbl");
	lua_getfield (L, -1, "id");
	lua_Integer id = lua_tointeger(L, -1);
	std::cout << id << std::endl;

	lua_getglobal (L, "tbl");
	lua_getfield (L, -1, "score");
	lua_Number score = lua_tonumber(L, -1);
	std::cout << score << std::endl;

	lua_getglobal (L, "add");
	lua_pushinteger(L, id);
	lua_pushnumber(L, score);
	lua_call(L, 2, 1);
	if (lua_isnumber(L, -1))
	{
		lua_Number result = lua_tonumber(L, -1);
		std::cout << result << std::endl;
	}

	lua_close(L);
	return 0;
}
