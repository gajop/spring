/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */


#include "LuaVBOs.h"

#include "LuaInclude.h"

#include "LuaHandle.h"
#include "LuaHashString.h"
#include "LuaUtils.h"

#include "Rendering/GL/myGL.h"
#include "Rendering/GL/VAO.h"

/******************************************************************************/
/******************************************************************************/

LuaVBOs::~LuaVBOs()
{
	for (const VBO* vbo: vbos) {
		delete vbo;
	}
}


/******************************************************************************/
/******************************************************************************/

bool LuaVBOs::PushEntries(lua_State* L)
{
	CreateMetatable(L);

	REGISTER_LUA_CFUNC(CreateVBO);
	// REGISTER_LUA_CFUNC(DeleteVBO);

	return true;
}


bool LuaVBOs::CreateMetatable(lua_State* L)
{
	luaL_newmetatable(L, "VBO");
	HSTR_PUSH_CFUNC(L, "__gc",        meta_gc);
	HSTR_PUSH_CFUNC(L, "__index",     meta_index);
	HSTR_PUSH_CFUNC(L, "__newindex",  meta_newindex);
	lua_pop(L, 1);
	return true;
}


/******************************************************************************/
/******************************************************************************/

const VBO* LuaVBOs::GetLuaVBO(lua_State* L, int index)
{
	return static_cast<VBO*>(LuaUtils::GetUserData(L, index, "VBO"));
}

/******************************************************************************/
/******************************************************************************/

int LuaVBOs::meta_gc(lua_State* L)
{
	VBO* vbo = static_cast<VBO*>(luaL_checkudata(L, 1, "VBO"));
	// vbo->Free(L);
	delete vbo;
	return 0;
}


int LuaVBOs::meta_index(lua_State* L)
{
	const VBO* vbo = static_cast<VBO*>(luaL_checkudata(L, 1, "VBO"));
	const std::string& key = luaL_checkstring(L, 2);

	// if (key ==  "valid") { lua_pushboolean(L, glIsRenderbufferEXT(vbo->id)); return 1; }
	// if (key == "target") { lua_pushnumber(L, vbo->target); return 1; }
	// if (key == "format") { lua_pushnumber(L, vbo->format); return 1; }
	// if (key ==  "xsize") { lua_pushnumber(L, vbo->xsize ); return 1; }
	// if (key ==  "ysize") { lua_pushnumber(L, vbo->ysize ); return 1; }

	return 0;
}


int LuaVBOs::meta_newindex(lua_State* L)
{
	return 0;
}


/******************************************************************************/
/******************************************************************************/

int LuaVBOs::CreateVBO(lua_State* L)
{
	VBO* vboPtr = static_cast<VBO*>(lua_newuserdata(L, sizeof(VBO)));
	vboPtr->Generate();

	// vbo.xsize = (GLsizei)luaL_checknumber(L, 1);
	// vbo.ysize = (GLsizei)luaL_checknumber(L, 2);
	// vbo.target = GL_RENDERBUFFER_EXT;
	// vbo.format = GL_RGBA;
	//
	// const int table = 3;
	// if (lua_istable(L, table)) {
	// 	lua_getfield(L, table, "target");
	// 	if (lua_isnumber(L, -1)) {
	// 		vbo.target = (GLenum)lua_tonumber(L, -1);
	// 	}
	// 	lua_pop(L, 1);
	// 	lua_getfield(L, table, "format");
	// 	if (lua_isnumber(L, -1)) {
	// 		vbo.format = (GLenum)lua_tonumber(L, -1);
	// 	}
	// 	lua_pop(L, 1);
	// }

	luaL_getmetatable(L, "VBO");
	lua_setmetatable(L, -2);

	// if (vboPtr->id != 0) {
		LuaVBOs& activeVBOs = CLuaHandle::GetActiveVBOs(L);
		auto& vbos = activeVBOs.vbos;

		vbos.push_back(vboPtr);
	// 	vboPtr->index = vbos.size() - 1;
	// }

	return 1;
}

//
// int LuaVBOs::DeleteVBO(lua_State* L)
// {
// 	if (lua_isnil(L, 1)) {
// 		return 0;
// 	}
// 	VBO* vbo = static_cast<VBO*>(luaL_checkudata(L, 1, "VBO"));
// 	vbo->Free(L);
// 	return 0;
// }


/******************************************************************************/
/******************************************************************************/
