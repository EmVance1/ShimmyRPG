#include "pch.h"
#include "conversions.h"


void lua_pushvec2f(lua_State* L, const sf::Vector2f& vec) {
    lua_newtable(L);

    lua_pushstring(L, "x");
    lua_pushnumber(L, vec.x);
    lua_settable(L, -3);

    lua_pushstring(L, "y");
    lua_pushnumber(L, vec.y);
    lua_settable(L, -3);
}

void lua_pushvec2i(lua_State* L, const sf::Vector2i& vec) {
    lua_newtable(L);

    lua_pushstring(L, "x");
    lua_pushnumber(L, vec.x);
    lua_settable(L, -3);

    lua_pushstring(L, "y");
    lua_pushnumber(L, vec.y);
    lua_settable(L, -3);
}

void lua_pushvec2u(lua_State* L, const sf::Vector2u& vec) {
    lua_newtable(L);

    lua_pushstring(L, "x");
    lua_pushnumber(L, vec.x);
    lua_settable(L, -3);

    lua_pushstring(L, "y");
    lua_pushnumber(L, vec.y);
    lua_settable(L, -3);
}

void lua_pushcolor(lua_State* L, const sf::Color& color) {
    lua_newtable(L);

    lua_pushstring(L, "r");
    lua_pushnumber(L, color.r);
    lua_settable(L, -3);

    lua_pushstring(L, "g");
    lua_pushnumber(L, color.g);
    lua_settable(L, -3);

    lua_pushstring(L, "b");
    lua_pushnumber(L, color.b);
    lua_settable(L, -3);

    lua_pushstring(L, "a");
    lua_pushnumber(L, color.a);
    lua_settable(L, -3);
}

void lua_pushvertex(lua_State* L, const sf::Vertex& vert) {
    lua_newtable(L);

    lua_pushstring(L, "position");
    lua_pushvec2f(L, vert.position);
    lua_settable(L, -3);

    lua_pushstring(L, "color");
    lua_pushcolor(L, vert.color);
    lua_settable(L, -3);

    lua_pushstring(L, "texcoord");
    lua_pushvec2f(L, vert.texCoords);
    lua_settable(L, -3);
}


sf::Vector2f lua_tovec2f(lua_State* L, int idx) {
    lua_pushstring(L, "x");
    lua_gettable(L, idx);
    const float x = static_cast<float>(lua_tonumber(L, -1));
    lua_pop(L, 1);

    lua_pushstring(L, "y");
    lua_gettable(L, idx);
    const float y = static_cast<float>(lua_tonumber(L, -1));
    lua_pop(L, 1);

    return sf::Vector2f(x, y);
}

sf::Vector2i lua_tovec2i(lua_State* L, int idx) {
    lua_pushstring(L, "x");
    lua_gettable(L, idx);
    const int x = static_cast<int>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_pushstring(L, "y");
    lua_gettable(L, idx);
    const int y = static_cast<int>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    return sf::Vector2i(x, y);
}

sf::Vector2u lua_tovec2u(lua_State* L, int idx) {
    lua_pushstring(L, "x");
    lua_gettable(L, idx);
    const uint32_t x = static_cast<uint32_t>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_pushstring(L, "y");
    lua_gettable(L, idx);
    const uint32_t y = static_cast<uint32_t>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    return sf::Vector2u(x, y);
}

sf::Color lua_tocolor(lua_State* L, int idx) {
    lua_pushinteger(L, 1);
    lua_gettable(L, idx);
    const uint8_t r = static_cast<uint8_t>(lua_tonumber(L, -1));
    lua_pop(L, 1);

    lua_pushinteger(L, 2);
    lua_gettable(L, idx);
    const uint8_t g = static_cast<uint8_t>(lua_tonumber(L, -1));
    lua_pop(L, 1);

    lua_pushinteger(L, 3);
    lua_gettable(L, idx);
    const uint8_t b = static_cast<uint8_t>(lua_tonumber(L, -1));
    lua_pop(L, 1);

    // lua_pushstring(L, "a");
    // lua_gettable(L, idx);
    // const uint32_t a = static_cast<uint32_t>(lua_tointeger(L, -1));
    // lua_pop(L, 1);

    return sf::Color(r, g, b);
}

sf::Vertex lua_tovertex(lua_State* L, int idx) {
    lua_pushstring(L, "position");
    lua_gettable(L, idx);
    const sf::Vector2f pos = lua_tovec2f(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, "color");
    lua_gettable(L, idx);
    const sf::Color col = lua_tocolor(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, "texcoord");
    lua_gettable(L, idx);
    const sf::Vector2f tex = lua_tovec2f(L, -1);
    lua_pop(L, 1);

    return sf::Vertex(pos, col, tex);
}

