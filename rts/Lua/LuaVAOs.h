/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#ifndef LUA_VAOS_H
#define LUA_VAOS_H

#include <vector>

struct lua_State;

struct VAO;
class LuaVAOs {
	public:
		LuaVAOs() { vaos.reserve(8); }
		~LuaVAOs();

		void Clear() { vaos.clear(); }

		static bool PushEntries(lua_State* L);

		struct LuaVAO;
		static const LuaVAO* GetLuaVAO(lua_State* L, int index);

	public:
		struct LuaVAO {
			LuaVAO() : index(-1u) {}

			void Init();
			void Free(lua_State* L);

			unsigned int index; // into LuaVAOs::vaos
			VAO* vao;
		};

	private:
		std::vector<LuaVAO*> vaos;

	private: // helpers
		static bool CreateMetatable(lua_State* L);

	private: // metatable methods
		static int meta_gc(lua_State* L);
		static int meta_index(lua_State* L);
		static int meta_newindex(lua_State* L);
		static int meta_Bind(lua_State* L);
		static int meta_Unbind(lua_State* L);

	private:
		static int CreateVAO(lua_State* L);
};


#endif /* LUA_VAOS_H */
