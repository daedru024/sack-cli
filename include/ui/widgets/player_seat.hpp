#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "ui/common/ui_common.hpp"
#include "ui/widgets/color_selector.hpp"   // 為了 PLAYER_COLORS

struct PlayerSeat {
    std::string name;
    int         colorIndex = -1;
    bool        isSelf     = false;

    sf::Text    text;
    sf::RectangleShape background;

    void init(const sf::Font& font,
              const std::string& nm,
              int colorIdx,
              bool selfFlag,
              sf::Vector2f pos)
    {
        name       = nm;
        colorIndex = colorIdx;
        isSelf     = selfFlag;

        text.setFont(font);
        text.setCharacterSize(24);
        text.setString(toUtf32(displayName()));
        text.setFillColor(colorIndex >= 0 && colorIndex < (int)PLAYER_COLORS.size()
                          ? PLAYER_COLORS[colorIndex]
                          : sf::Color::White);
        text.setOutlineColor(sf::Color::Black);
        text.setOutlineThickness(2.f);

        background.setFillColor(sf::Color(0, 0, 0, 150));

        auto b = text.getLocalBounds();
        background.setSize({b.width + 10.f, b.height + 10.f}); 
        
        background.setOrigin(background.getSize().x / 2.f, 
                              background.getSize().y / 2.f);
        
        background.setPosition(pos); // 底框與文字中心對齊
        text.setOrigin(b.left + b.width / 2.f,
                       b.top  + b.height / 2.f);
        text.setPosition(pos);
    }

    std::string displayName() const {
        if (isSelf) return name + " [You]";
        return name;
    }

    void draw(sf::RenderWindow& win) const {
        win.draw(background);
        win.draw(text);
    }
};
