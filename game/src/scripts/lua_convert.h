#pragma once
#include <SFML/Graphics.hpp>
extern "C" {
#include <lua/lua.h>
}


void lua_pushvec2f(lua_State* L, const sf::Vector2f& vec);
void lua_pushvec2i(lua_State* L, const sf::Vector2i& vec);
void lua_pushvec2u(lua_State* L, const sf::Vector2u& vec);
void lua_pushcolor(lua_State* L, const sf::Color& color);
void lua_pushvertex(lua_State* L, const sf::Vertex& vert);

sf::Vector2f lua_tovec2f(lua_State* L, int idx);
sf::Vector2i lua_tovec2i(lua_State* L, int idx);
sf::Vector2u lua_tovec2u(lua_State* L, int idx);
sf::Color lua_tocolor(lua_State* L, int idx);
sf::Vertex lua_tovertex(lua_State* L, int idx);

