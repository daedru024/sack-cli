#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "ui/common/ui_common.hpp"
#include "ui/widgets/game_cards.hpp"

class CardWidget {
public:
    int    cardId = -1;
    bool   selected = false;

    sf::RectangleShape rect;
    sf::Text           text;

    CardWidget() {}

    CardWidget(const sf::Font& font,
               int cardId,
               sf::Vector2f pos,
               sf::Vector2f size)
    {
        init(font, cardId, pos, size);
    }

    void init(const sf::Font& font,
              int id,
              sf::Vector2f pos,
              sf::Vector2f size)
    {
        cardId = id;
        rect.setSize(size);
        rect.setPosition(pos);
        rect.setOutlineThickness(3);
        rect.setOutlineColor(sf::Color::Black);

        auto t = getCardType(id);
        rect.setFillColor(cardFillColor(t));

        text.setFont(font);
        text.setCharacterSize(28);
        text.setFillColor(sf::Color::Black);

        int v = cardValue(id);
        text.setString(std::to_string(v));
        centerText();
    }

    void setPosition(sf::Vector2f pos) {
        rect.setPosition(pos);
        centerText();
    }

    void setSize(sf::Vector2f size) {
        rect.setSize(size);
        centerText();
    }

    void setSelected(bool s) {
        selected = s;
        rect.setOutlineThickness(selected ? 5.f : 3.f);
        rect.setOutlineColor(selected ? sf::Color(0,120,255) : sf::Color::Black);
    }

    bool hitTest(sf::Vector2f p) const {
        return rect.getGlobalBounds().contains(p);
    }

    void draw(sf::RenderWindow& win) const {
        win.draw(rect);
        win.draw(text);
    }

private:
    void centerText() {
        sf::FloatRect tb = text.getLocalBounds();
        sf::Vector2f   p = rect.getPosition();
        sf::Vector2f   s = rect.getSize();

        float cx = p.x + s.x / 2.f;
        float cy = p.y + s.y / 2.f;

        text.setOrigin(tb.left + tb.width / 2.f,
                       tb.top  + tb.height / 2.f);
        text.setPosition(cx, cy);
    }
};
