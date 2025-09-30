#include "pch.h"
#include "gui/bases/container.h"


namespace gui {

class Grid : public Container {
private:
    sf::Vector2f m_blocksize;

public:
    Grid(const Position& position, const sf::Vector2f& size, const Style& style);
    Grid(const Position& position, const sf::Vector2f& size, const Style& style, const std::vector<std::string>& values);

    static std::shared_ptr<Grid> create(const Position& position, const sf::Vector2f& size, const Style& style) {
        return std::make_shared<Grid>(position, size, style);
    }
    static std::shared_ptr<Grid> create(const Position& position, const sf::Vector2f& size, const Style& style, const std::vector<std::string>& values) {
        return std::make_shared<Grid>(position, size, style, values);
    }

    std::shared_ptr<Grid> add_button(const std::string& value);
    std::shared_ptr<Grid> add_button(const std::string& value, std::function<void()> callback);
    void clear() { Container::clear(); set_size({get_size().x, 0}); }
};

}

