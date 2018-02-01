/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */


#include "LuaVAOs.h"

#include "LuaInclude.h"

#include "LuaHandle.h"
#include "LuaHashString.h"
#include "LuaUtils.h"

#include "Rendering/GL/VAO.h"


/******************************************************************************/
/******************************************************************************/

LuaVAOs::~LuaVAOs()
{
	// for (const VAO* vao: vaos) {
	// 	delete vao;
	// }
}


/******************************************************************************/
/******************************************************************************/

bool LuaVAOs::PushEntries(lua_State* L)
{
	CreateMetatable(L);

	REGISTER_LUA_CFUNC(CreateVAO);

	return true;
}


bool LuaVAOs::CreateMetatable(lua_State* L)
{
	luaL_newmetatable(L, "VAO");

	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	HSTR_PUSH_CFUNC(L, "__gc",        meta_gc);
	// HSTR_PUSH_CFUNC(L, "__index",     meta_index);
	HSTR_PUSH_CFUNC(L, "__newindex",  meta_newindex);
	HSTR_PUSH_CFUNC(L, "Bind",        meta_Bind);
	HSTR_PUSH_CFUNC(L, "Unbind",      meta_Unbind);
	lua_pop(L, 1);
	return true;
}


/******************************************************************************/
/******************************************************************************/

const LuaVAOs::LuaVAO* LuaVAOs::GetLuaVAO(lua_State* L, int index)
{
	return static_cast<LuaVAO*>(LuaUtils::GetUserData(L, index, "VAO"));
}


/******************************************************************************/
/******************************************************************************/

//
// void LuaVAOs::VAO::Init()
// {
// 	index  = -1u;
// 	id     = 0;
// 	target = GL_RENDERBUFFER_EXT;
// 	format = GL_RGBA;
// 	xsize  = 0;
// 	ysize  = 0;
// }


void LuaVAOs::LuaVAO::Free(lua_State* L)
{
	// if (id == 0)
	// 	return;
	//
	// glDeleteRenderbuffersEXT(1, &id);
	// id = 0;
	printf("VAO Index: %d\n", index);

	{
		// get rid of the userdatum
		LuaVAOs& activeVAOs = CLuaHandle::GetActiveVAOs(L);
		auto& vaos = activeVAOs.vaos;

		assert(index < vaos.size());
		assert(vaos[index] == this);

		vaos[index] = vaos.back();
		vaos[index]->index = index;
		vaos.pop_back();
	}
}


/******************************************************************************/
/******************************************************************************/

int LuaVAOs::meta_gc(lua_State* L)
{
	LuaVAO* luaVAO = static_cast<LuaVAO*>(luaL_checkudata(L, 1, "VAO"));
	luaVAO->Free(L);
	return 0;
}


int LuaVAOs::meta_index(lua_State* L)
{
	// const VAO* vao = static_cast<VAO*>(luaL_checkudata(L, 1, "VAO"));
	// const std::string& key = luaL_checkstring(L, 2);
	//
	// if (key ==  "valid") { lua_pushboolean(L, glIsRenderbufferEXT(vao->id)); return 1; }
	// if (key == "target") { lua_pushnumber(L, vao->target); return 1; }
	// if (key == "format") { lua_pushnumber(L, vao->format); return 1; }
	// if (key ==  "xsize") { lua_pushnumber(L, vao->xsize ); return 1; }
	// if (key ==  "ysize") { lua_pushnumber(L, vao->ysize ); return 1; }
	//
	return 0;
}


int LuaVAOs::meta_newindex(lua_State* L)
{
	return 0;
}

int LuaVAOs::meta_Bind(lua_State* L)
{
	LuaVAO* luaVAO = static_cast<LuaVAO*>(luaL_checkudata(L, 1, "VAO"));
	VAO* vao = luaVAO->vao;
	vao->Bind();
	return 0;
}

int LuaVAOs::meta_Unbind(lua_State* L)
{
	LuaVAO* luaVAO = static_cast<LuaVAO*>(luaL_checkudata(L, 1, "VAO"));
	VAO* vao = luaVAO->vao;
	vao->Unbind();
	return 0;
}


/******************************************************************************/
/******************************************************************************/

int LuaVAOs::CreateVAO(lua_State* L)
{
	LuaVAO* luaVao = static_cast<LuaVAO*>(lua_newuserdata(L, sizeof(LuaVAO)));
	luaVao->vao = new VAO();
	luaVao->vao->Generate();

	luaL_getmetatable(L, "VAO");
	lua_setmetatable(L, -2);

	// if (vaoPtr->id != 0) {
		LuaVAOs& activeVAOs = CLuaHandle::GetActiveVAOs(L);
		auto& vaos = activeVAOs.vaos;

		vaos.push_back(luaVao);
		luaVao->index = vaos.size() - 1;
	// }

	return 1;
}


// int LuaVAOs::DeleteVAO(lua_State* L)
// {
// 	if (lua_isnil(L, 1)) {
// 		return 0;
// 	}
// 	VAO* vao = static_cast<VAO*>(luaL_checkudata(L, 1, "VAO"));
// 	vao->Free(L);
// 	return 0;
// }


/******************************************************************************/
/******************************************************************************/
