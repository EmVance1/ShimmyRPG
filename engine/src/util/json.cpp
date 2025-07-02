#include "pch.h"
#include "json.h"
#include <rapidjson/error/en.h>


sf::Vector2f json_to_vector2f(const rapidjson::Value& arr) {
    return sf::Vector2f(arr.GetArray()[0].GetFloat(), arr.GetArray()[1].GetFloat());
}

sf::Vector2i json_to_vector2i(const rapidjson::Value& arr) {
    return sf::Vector2i(arr.GetArray()[0].GetInt(), arr.GetArray()[1].GetInt());
}

sf::Vector2u json_to_vector2u(const rapidjson::Value& arr) {
    return sf::Vector2u(arr.GetArray()[0].GetUint(), arr.GetArray()[1].GetUint());
}


sf::FloatRect json_to_floatrect(const rapidjson::Value& arr) {
    return sf::FloatRect(
        { arr.GetArray()[0].GetFloat(),
          arr.GetArray()[1].GetFloat() },
        { arr.GetArray()[2].GetFloat(),
          arr.GetArray()[3].GetFloat() }
    );
}

sf::IntRect json_to_intrect(const rapidjson::Value& arr) {
    return sf::IntRect(
        { arr.GetArray()[0].GetInt(),
          arr.GetArray()[1].GetInt() },
        { arr.GetArray()[2].GetInt(),
          arr.GetArray()[3].GetInt() }
    );
}


sf::Color json_to_color(const rapidjson::Value& arr) {
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


rapidjson::Document load_json_from_file(const std::string& filename) {
    FILE* fp = nullptr;
    fopen_s(&fp, filename.c_str(), "rb");
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    rapidjson::Document doc;
#ifdef DEBUG
    rapidjson::ParseResult ok = doc.ParseStream(is);
    if (!ok) {
        throw std::invalid_argument(std::string("invalid json document - ")
                + rapidjson::GetParseError_En(ok.Code()) + " (" + std::to_string(ok.Offset()) + ")");
    }
#else
    doc.ParseStream(is);
#endif
    fclose(fp);
    return doc;
}

