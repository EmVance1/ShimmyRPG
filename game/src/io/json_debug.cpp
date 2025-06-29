#include "pch.h"
#include "json_debug.h"


const char* json_debug_get_str(const rapidjson::Value& object, const char* name) {
    if (!object.HasMember(name)) {
        throw std::invalid_argument(std::string("invalid json access - object has no member '") + name + "");
    }
    const auto& e = object[name];
    if (!e.IsString()) {
        throw std::invalid_argument(std::string("invalid json access - object member '") + name + "' is not of expected type");
    }
    return e.GetString();
}
size_t json_debug_get_strlen(const rapidjson::Value& object, const char* name) {
    if (!object.HasMember(name)) {
        throw std::invalid_argument(std::string("invalid json access - object has no member '") + name + "'");
    }
    const auto& e = object[name];
    if (!e.IsString()) {
        throw std::invalid_argument(std::string("invalid json access - object member '") + name + "' is not of expected type");
    }
    return e.GetStringLength();
}
bool json_debug_get_bool(const rapidjson::Value& object, const char* name) {
    if (!object.HasMember(name)) {
        throw std::invalid_argument(std::string("invalid json access - object has no member '") + name + "'");
    }
    const auto& e = object[name];
    if (!e.IsBool()) {
        throw std::invalid_argument(std::string("invalid json access - object member '") + name + "' is not of expected type");
    }
    return e.GetBool();
}
bool json_debug_is_null(const rapidjson::Value& object, const char* name) {
    if (!object.HasMember(name)) {
        throw std::invalid_argument(std::string("invalid json access - object has no member '") + name + "'");
    }
    return object[name].IsNull();
}
const rapidjson::Value& json_debug_get(const rapidjson::Value& object, const char* name) {
    if (!object.HasMember(name)) {
        throw std::invalid_argument(std::string("invalid json access - object has no member '") + name + "'");
    }
    return object[name];
}


