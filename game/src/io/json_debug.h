#pragma once
#include <rapidjson/document.h>
#include <stdexcept>


#ifdef DEBUG

template<typename T>
const T& json_debug_get_number(const rapidjson::Value& object, const char* name) {
    if (!object.HasMember(name)) {
        throw std::invalid_argument(std::string("invalid json access - object has no member '") + name + "'");
    }
    const auto& e = object[name];
    if (!e.IsNumber()) {
        throw std::invalid_argument(std::string("invalid json access - object member '") + name + "' is not of expected type");
    }
    return e.Get<T>();
}
const char* json_debug_get_str(const rapidjson::Value& object, const char* name);
size_t json_debug_get_strlen(const rapidjson::Value& object, const char* name);
bool json_debug_get_bool(const rapidjson::Value& object, const char* name);
bool json_debug_is_true(const rapidjson::Value& object, const char* name);
bool json_debug_is_false(const rapidjson::Value& object, const char* name);
bool json_debug_is_null(const rapidjson::Value& object, const char* name);
const rapidjson::Value& json_debug_get(const rapidjson::Value& object, const char* name);

#define JSON_GET_INT(object, name)    json_debug_get_number<int32_t>(object, name)
#define JSON_GET_INT64(object, name)  json_debug_get_number<int64_t>(object, name)
#define JSON_GET_UINT(object, name)   json_debug_get_number<uint32_t>(object, name)
#define JSON_GET_UINT64(object, name) json_debug_get_number<uint64_t>(object, name)
#define JSON_GET_FLOAT(object, name)  json_debug_get_number<float>(object, name)
#define JSON_GET_DOUBLE(object, name) json_debug_get_number<double>(object, name)
#define JSON_GET_STR(object, name)    json_debug_get_str(object, name)
#define JSON_GET_STRLEN(object, name) json_debug_get_strlen(object, name)
#define JSON_GET_BOOL(object, name)   json_debug_get_bool(object, name)
#define JSON_IS_TRUE(object, name)    json_debug_get_bool(object, name)
#define JSON_IS_FALSE(object, name)   !json_debug_get_bool(object, name)
#define JSON_IS_NULL(object, name)    json_debug_is_null(object, name)
#define JSON_GET_ARRAY(object, name)  object[name].GetArray()
#define JSON_GET(object, name)        json_debug_get(object, name)

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

