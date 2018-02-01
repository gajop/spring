/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#ifndef LUA_VBOS_H
#define LUA_VBOS_H

#include <vector>

struct lua_State;

class VBO;
class LuaVBOs {
	public:
		LuaVBOs() { vbos.reserve(8); }
		~LuaVBOs();

		void Clear() { vbos.clear(); }

		static bool PushEntries(lua_State* L);

		struct LuaVBO;
		static const LuaVBO* GetLuaVBO(lua_State* L, int index);

	public:
		struct LuaVBO {
			LuaVBO() : index(-1u) {}

			void Init();
			void Free(lua_State* L);

			unsigned int index; // into LuaVAOs::vaos
			VBO* vbo;
		};

	private:
		std::vector<LuaVBO*> vbos;

	private: // helpers
		static bool CreateMetatable(lua_State* L);

	private: // metatable methods
		static int meta_gc(lua_State* L);
		static int meta_index(lua_State* L);
		static int meta_newindex(lua_State* L);
		static int meta_Bind(lua_State* L);
		static int meta_Unbind(lua_State* L);
		static int meta_Set(lua_State* L);

	private:
		static int CreateVBO(lua_State* L);
		// static int DeleteVBO(lua_State* L);
};


#endif /* LUA_VBOS_H */
