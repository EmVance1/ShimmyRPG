#pragma once
#include <SFML/Graphics.hpp>
#include "bases/container.h"


namespace gui {

class VerticalList : public Container {
private:
    std::vector<std::string> m_order;

public:
    VerticalList(const Position& position, const Sizing& sizing, const Style& style);
    VerticalList(const Position& position, const Sizing& sizing, const Style& style, const std::vector<std::string>& values);

    static std::shared_ptr<VerticalList> create(const Position& position, const Sizing& sizing, const Style& style) {
        return std::make_shared<VerticalList>(position, sizing, style);
    }

    virtual void add_widget(const std::string& name, const std::shared_ptr<Widget>& widget, bool top = false) override;
    virtual std::string add_widget(const std::shared_ptr<Widget>& widget, bool top = false) override;
    virtual std::shared_ptr<Widget> remove_widget(const std::string& id) override;
    virtual void clear() override;
    void refresh();
};

}
