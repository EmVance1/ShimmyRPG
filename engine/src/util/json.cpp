#include "pch.h"
#include "json.h"
#include <rapidjson/reader.h>
#include <rapidjson/error/en.h>


namespace shmy { namespace json {

sf::Vector2f into_vector2f(const rapidjson::Value& arr) {
    return sf::Vector2f(arr.GetArray()[0].GetFloat(), arr.GetArray()[1].GetFloat());
}

sf::Vector2i into_vector2i(const rapidjson::Value& arr) {
    return sf::Vector2i(arr.GetArray()[0].GetInt(), arr.GetArray()[1].GetInt());
}

sf::Vector2u into_vector2u(const rapidjson::Value& arr) {
    return sf::Vector2u(arr.GetArray()[0].GetUint(), arr.GetArray()[1].GetUint());
}


sf::FloatRect into_floatrect(const rapidjson::Value& arr) {
    return sf::FloatRect(
        { arr.GetArray()[0].GetFloat(),
          arr.GetArray()[1].GetFloat() },
        { arr.GetArray()[2].GetFloat(),
          arr.GetArray()[3].GetFloat() }
    );
}

sf::IntRect into_intrect(const rapidjson::Value& arr) {
    return sf::IntRect(
        { arr.GetArray()[0].GetInt(),
          arr.GetArray()[1].GetInt() },
        { arr.GetArray()[2].GetInt(),
          arr.GetArray()[3].GetInt() }
    );
}


sf::Color into_color(const rapidjson::Value& arr) {
    if (arr.GetArray().Size() == 3) {
        return sf::Color((uint8_t)arr.GetArray()[0].GetUint(),
                         (uint8_t)arr.GetArray()[1].GetUint(),
                         (uint8_t)arr.GetArray()[2].GetUint());
    } else {
        return sf::Color((uint8_t)arr.GetArray()[0].GetUint(),
                         (uint8_t)arr.GetArray()[1].GetUint(),
                         (uint8_t)arr.GetArray()[2].GetUint(),
                         (uint8_t)arr.GetArray()[3].GetUint());
    }
}


#ifdef _WIN32
#define os_fopen(filename, mode) _wfopen(filename.c_str(), L##mode);
#else
#define os_fopen(filename, mode) fopen((const char*)filename.u8string().c_str(), mode);
#endif

rapidjson::Document load_from_file(const std::fs::path& filename) {
    FILE* fp = os_fopen(filename, "rb");
    if (!fp) {
        std::cerr << "failed to open json document: " << filename.string() << '\n';
        exit(1);
    }

    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    rapidjson::Document doc;
#ifdef DEBUG
    constexpr auto FLAGS = rapidjson::kParseValidateEncodingFlag|rapidjson::kParseTrailingCommasFlag|rapidjson::kParseCommentsFlag;
    rapidjson::ParseResult ok = doc.ParseStream<FLAGS>(is);
    if (!ok) {
        throw std::invalid_argument(std::string("invalid json document - ")
                + rapidjson::GetParseError_En(ok.Code()) + " (" + std::to_string(ok.Offset()) + ")");
    }
#else
    constexpr auto FLAGS = rapidjson::kParseTrailingCommasFlag|rapidjson::kParseCommentsFlag;
    doc.ParseStream<FLAGS>(is);
#endif
    fclose(fp);
    return doc;
}


#ifdef DEBUG

const char* debug_get_str(const rapidjson::Value& object, const char* name) {
    if (!object.HasMember(name)) {
        throw std::invalid_argument(std::string("invalid json access - object has no member '") + name + "");
    }
    const auto& e = object[name];
    if (!e.IsString()) {
        throw std::invalid_argument(std::string("invalid json access - object member '") + name + "' is not of expected type");
    }
    return e.GetString();
}
size_t debug_get_strlen(const rapidjson::Value& object, const char* name) {
    if (!object.HasMember(name)) {
        throw std::invalid_argument(std::string("invalid json access - object has no member '") + name + "'");
    }
    const auto& e = object[name];
    if (!e.IsString()) {
        throw std::invalid_argument(std::string("invalid json access - object member '") + name + "' is not of expected type");
    }
    return e.GetStringLength();
}
bool debug_get_bool(const rapidjson::Value& object, const char* name) {
    if (!object.HasMember(name)) {
        throw std::invalid_argument(std::string("invalid json access - object has no member '") + name + "'");
    }
    const auto& e = object[name];
    if (!e.IsBool()) {
        throw std::invalid_argument(std::string("invalid json access - object member '") + name + "' is not of expected type");
    }
    return e.GetBool();
}
bool debug_is_null(const rapidjson::Value& object, const char* name) {
    if (!object.HasMember(name)) {
        throw std::invalid_argument(std::string("invalid json access - object has no member '") + name + "'");
    }
    return object[name].IsNull();
}
const rapidjson::Value& debug_get(const rapidjson::Value& object, const char* name) {
    if (!object.HasMember(name)) {
        throw std::invalid_argument(std::string("invalid json access - object has no member '") + name + "'");
    }
    return object[name];
}

#endif

} }

