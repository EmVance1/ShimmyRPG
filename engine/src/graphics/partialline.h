#pragma once
#include <SFML/Graphics.hpp>


class PartialLine : public sf::Drawable {
private:
    std::vector<sf::Vertex> m_data;
    size_t m_count;

public:
    explicit PartialLine(size_t vertexCount = 0) {
        m_data.resize(vertexCount);
    }
    template<typename T>
    static PartialLine from_path(const std::vector<sf::Vector2<T>>& points, float scale, const sf::Vector2f& offset) {
        PartialLine result;
        result.m_data.resize(points.size());
        for (size_t i = 0; i < points.size(); i++) {
            result.m_data[points.size() - i - 1] = sf::Vertex(sf::Vector2f(points[i]) * scale + offset, sf::Color::White);
        }
        return result;
    }

    [[nodiscard]] size_t get_count() const {
        return m_count;
    }
    void set_count(size_t count) {
        m_count = count;
    }
    [[nodiscard]] size_t size() const {
        return m_data.size();
    }
    [[nodiscard]] sf::Vertex& operator[](size_t index) {
        return m_data[index];
    }
    [[nodiscard]] const sf::Vertex& operator[](size_t index) const {
        return m_data[index];
    }
    void clear() {
        m_data.clear();
    }
    void resize(size_t vertexCount) {
        m_data.resize(vertexCount);
    }
    void append(const sf::Vertex& vertex) {
        m_data.push_back(vertex);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        const auto count = std::min(m_data.size(), m_count);
        target.draw(m_data.data(), count, sf::PrimitiveType::LineStrip, states);
    }
};

