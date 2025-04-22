#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>


class PartialLine : public sf::Drawable, public sf::Transformable {
private:
    std::vector<sf::Vertex> m_data;
    size_t m_begin = 0;
    size_t m_end = 0;

public:
    explicit PartialLine(size_t vertexCount = 0) {
        m_data.resize(vertexCount);
    }
    template<typename T>
    static PartialLine from_path(const std::vector<sf::Vector2<T>>& points) {
        PartialLine result;
        result.m_data.resize(points.size());
        for (size_t i = 0; i < points.size(); i++) {
            result.m_data[points.size() - i - 1] = sf::Vertex(sf::Vector2f(points[i]), sf::Color::White);
        }
        return result;
    }

    [[nodiscard]] size_t get_start() const {
        return m_begin;
    }
    void set_start(size_t start) {
        m_begin = start;
    }
    [[nodiscard]] size_t get_count() const {
        return m_end - m_begin;;
    }
    void set_count(size_t count) {
        m_end = m_begin + count;
    }
    [[nodiscard]] size_t get_size() const {
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
        m_begin = 0;
        m_end = 0;
    }
    void resize(size_t vertexCount) {
        m_data.resize(vertexCount);
    }
    void append(const sf::Vertex& vertex) {
        m_data.push_back(vertex);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        if (m_data.size() == 0 || m_data.size() < m_begin) { return; }
        const auto count = std::min(m_data.size() - m_begin, get_count());
        states.transform *= getTransform();
        target.draw(m_data.data() + m_begin, count, sf::PrimitiveType::LineStrip, states);
    }
};

