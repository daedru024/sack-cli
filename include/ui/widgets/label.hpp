#ifndef LABEL_HPP
#define LABEL_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include "ui/common/ui_common.hpp"

class Label {
public:
    sf::Text text;

    Label(const sf::Font* font,
          const std::string& str,
          float x, float y,
          int size,
          sf::Color col = sf::Color::Black,
          sf::Color outline = sf::Color::Transparent,
          float outlineThickness = 0.f)
    {
        text.setFont(*font);
        text.setCharacterSize(size);

        text.setString(toUtf32(str));   // ★ 支援中文

        text.setFillColor(col);
        text.setOutlineColor(outline);
        text.setOutlineThickness(outlineThickness);

        text.setPosition(x, y);
    }

    void set(const std::string& str)
    {
        text.setString(toUtf32(str));   // ★ 支援中文
    }

    void centerText()
    {
        auto b = text.getLocalBounds();
        text.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
    }

    void draw(sf::RenderWindow& window)
    {
        window.draw(text);
    }
};

#endif
