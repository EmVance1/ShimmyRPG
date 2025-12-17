#pragma once
#include <SFML/Graphics.hpp>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <stdexcept>


namespace shmy { namespace json {

sf::Vector2f into_vector2f(const rapidjson::Value& arr);
sf::Vector2i into_vector2i(const rapidjson::Value& arr);
sf::Vector2u into_vector2u(const rapidjson::Value& arr);

sf::FloatRect into_floatrect(const rapidjson::Value& arr);
sf::IntRect into_intrect(const rapidjson::Value& arr);

sf::Color into_color(const rapidjson::Value& arr);

rapidjson::Document load_from_file(const std::filesystem::path& filename);

#ifdef SHMY_DEBUG

template<typename T>
T debug_get_number(const rapidjson::Value& object, const char* name) {
    if (!object.HasMember(name)) {
        throw std::invalid_argument(std::string("json object has no member '") + name + "'");
    }
    const auto& e = object[name];
    if (!e.IsNumber()) {
        throw std::invalid_argument(std::string("json object member '") + name + "' is not of expected type");
    }
    return e.Get<T>();
}
const char* debug_get_str(const rapidjson::Value& object, const char* name);
size_t debug_get_strlen(const rapidjson::Value& object, const char* name);
bool debug_get_bool(const rapidjson::Value& object, const char* name);
bool debug_is_true(const rapidjson::Value& object, const char* name);
bool debug_is_false(const rapidjson::Value& object, const char* name);
bool debug_is_null(const rapidjson::Value& object, const char* name);
const rapidjson::Value& debug_get(const rapidjson::Value& object, const char* name);

#endif

} }

#ifdef SHMY_DEBUG

#define JSON_GET_INT(object, name)    shmy::json::debug_get_number<int32_t>(object, name)
#define JSON_GET_INT64(object, name)  shmy::json::debug_get_number<int64_t>(object, name)
#define JSON_GET_UINT(object, name)   shmy::json::debug_get_number<uint32_t>(object, name)
#define JSON_GET_UINT64(object, name) shmy::json::debug_get_number<uint64_t>(object, name)
#define JSON_GET_FLOAT(object, name)  shmy::json::debug_get_number<float>(object, name)
#define JSON_GET_DOUBLE(object, name) shmy::json::debug_get_number<double>(object, name)
#define JSON_GET_STR(object, name)    shmy::json::debug_get_str(object, name)
#define JSON_GET_STRLEN(object, name) shmy::json::debug_get_strlen(object, name)
#define JSON_GET_BOOL(object, name)   shmy::json::debug_get_bool(object, name)
#define JSON_IS_TRUE(object, name)    shmy::json::debug_get_bool(object, name)
#define JSON_IS_FALSE(object, name)   !shmy::json::debug_get_bool(object, name)
#define JSON_IS_NULL(object, name)    shmy::json::debug_is_null(object, name)
#define JSON_GET_ARRAY(object, name)  object[name].GetArray()
#define JSON_GET(object, name)        shmy::json::debug_get(object, name)

#else

#define JSON_GET_INT(object, name)    object[name].GetInt()
#define JSON_GET_INT64(object, name)  object[name].GetInt64()
#define JSON_GET_UINT(object, name)   object[name].GetUint()
#define JSON_GET_UINT64(object, name) object[name].GetUint64()
#define JSON_GET_FLOAT(object, name)  object[name].GetFloat()
#define JSON_GET_DOUBLE(object, name) object[name].GetDouble()
#define JSON_GET_STR(object, name)    object[name].GetString()
#define JSON_GET_STRLEN(object, name) object[name].GetStringLength()
#define JSON_GET_BOOL(object, name)   object[name].GetBool()
#define JSON_IS_TRUE(object, name)    object[name].IsTrue()
#define JSON_IS_FALSE(object, name)   object[name].IsFalse()
#define JSON_IS_NULL(object, name)    object[name].IsNull()
#define JSON_GET_ARRAY(object, name)  object[name].GetArray()
#define JSON_GET(object, name)        object[name]

#endif

