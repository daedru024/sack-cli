#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class TextBox {
public:
    sf::RectangleShape box;
    sf::Text text;
    sf::Text placeholder;
    std::string buffer;
    bool focused  = false;
    bool disabled = false;

    TextBox(sf::Font* font,
            float x, float y,
            float w = 300.f,
            float h = 50.f,
            bool centered = false)
    {
        box.setSize({w, h});

        if (centered) {
            box.setOrigin(w / 2.f, h / 2.f);
            box.setPosition(x, y);
        } else {
            box.setPosition(x, y);
        }

        box.setFillColor(sf::Color::White);
        box.setOutlineThickness(2);
        box.setOutlineColor(sf::Color::Black);

        text.setFont(*font);
        text.setCharacterSize(28);
        text.setFillColor(sf::Color::Black);

        placeholder.setFont(*font);
        placeholder.setCharacterSize(24);
        placeholder.setFillColor(sf::Color(140, 140, 140));

        updateTextPosition();
        updatePlaceholderPosition();
    }

    void setPlaceholder(const std::string& s) {
        placeholder.setString(s);
        updatePlaceholderPosition();
    }

    void setDisabled(bool d) {
        disabled = d;
        if (disabled) {
            box.setFillColor(sf::Color(220, 220, 220));
            box.setOutlineColor(sf::Color(120, 120, 120));
        } else {
            box.setFillColor(sf::Color::White);
            box.setOutlineColor(sf::Color::Black);
        }
    }

    void handleEvent(const sf::Event& e, sf::RenderWindow& win) {
        if (disabled) return;

        // 滑鼠點擊 → 決定 focus
        if (e.type == sf::Event::MouseButtonPressed &&
            e.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2f world = win.mapPixelToCoords(
                { e.mouseButton.x, e.mouseButton.y }
            );
            if (box.getGlobalBounds().contains(world)) {
                focused = true;
                box.setOutlineColor(sf::Color(0, 120, 255));
            } else {
                focused = false;
                box.setOutlineColor(sf::Color::Black);
            }
        }

        if (!focused) return;

        // 文字輸入
        if (e.type == sf::Event::TextEntered) {
            const auto uni = e.text.unicode;

            if (uni == 8) { // Backspace
                if (!buffer.empty())
                    buffer.pop_back();
            } 
            else if (uni >= 32 && uni < 128) {
                // 只接受 ASCII
                buffer.push_back(static_cast<char>(uni));
            }

            text.setString(buffer);
            updateTextPosition();
        }
    }

    void draw(sf::RenderWindow& win) const {
        win.draw(box);

        if (buffer.empty() && !focused && !placeholder.getString().isEmpty())
            win.draw(placeholder);
        else
            win.draw(text);
    }

private:
    void updateTextPosition() {
        sf::FloatRect tb = text.getLocalBounds();
        sf::Vector2f pos = box.getPosition();
        sf::Vector2f origin = box.getOrigin();

        float cx, cy;
        if (origin.x == 0.f && origin.y == 0.f) {
            // 左上角模式：靠左內縮 10px
            cx = pos.x + 10.f;
            cy = pos.y + (box.getSize().y - tb.height) / 2.f - tb.top;
            text.setOrigin(0.f, 0.f);
            text.setPosition(cx, cy);
        } else {
            // 中心模式：置中
            cx = pos.x;
            cy = pos.y;
            text.setOrigin(tb.left + tb.width / 2.f,
                           tb.top  + tb.height / 2.f);
            text.setPosition(cx, cy);
        }
    }

    void updatePlaceholderPosition() {
        sf::FloatRect pb = placeholder.getLocalBounds();
        sf::Vector2f pos = box.getPosition();
        sf::Vector2f origin = box.getOrigin();

        float cx, cy;
        if (origin.x == 0.f && origin.y == 0.f) {
            cx = pos.x + 10.f;
            cy = pos.y + (box.getSize().y - pb.height) / 2.f - pb.top;
            placeholder.setOrigin(0.f, 0.f);
            placeholder.setPosition(cx, cy);
        } else {
            cx = pos.x;
            cy = pos.y;
            placeholder.setOrigin(pb.left + pb.width / 2.f,
                                  pb.top  + pb.height / 2.f);
            placeholder.setPosition(cx, cy);
        }
    }
};
