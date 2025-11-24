#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "ui/common/ui_common.hpp"  

class Button {
public:
    sf::RectangleShape shape;
    sf::Text text;
    bool disabled = false;

    Button() {}

    // x, y: 預設視為「左上角」
    // 若 centered = true，x, y 則視為「中心點」
    Button(sf::Font* font,
           const std::string& label,
           float x, float y,
           float w, float h,
           bool centered = false)
    {
        shape.setSize({w, h});

        if (centered) {
            shape.setOrigin(w / 2.f, h / 2.f);
            shape.setPosition(x, y);
        } else {
            shape.setPosition(x, y);
        }

        shape.setFillColor(sf::Color(220, 220, 220));
        shape.setOutlineThickness(3);
        shape.setOutlineColor(sf::Color(80, 80, 80));

        text.setFont(*font);
        text.setCharacterSize(32);
        text.setFillColor(sf::Color::Black);

        // ★★★ 支援中文：使用 UTF-32 字串 ★★★
        text.setString(toUtf32(label));

        centerText();
    }

    // ------------------------------------------------------
    // ✔ 動態更改文字（支援中文）
    // ------------------------------------------------------
    void setLabel(const std::string& s)
    {
        text.setString(toUtf32(s));
        centerText();
    }

    // ------------------------------------------------------
    // ✔ 動態更改字型（切換語系也可以）
    // ------------------------------------------------------
    void setFont(sf::Font* f)
    {
        text.setFont(*f);
        centerText();
    }

    // ------------------------------------------------------
    // ✔ Disabled 狀態
    // ------------------------------------------------------
    void setDisabled(bool d) {
        disabled = d;
        if (disabled) {
            shape.setFillColor(sf::Color(150, 150, 150));
            text.setFillColor(sf::Color(80, 80, 80));
        } else {
            shape.setFillColor(sf::Color(220, 220, 220));
            text.setFillColor(sf::Color::Black);
        }
    }

    // ------------------------------------------------------
    // ✔ Hover 效果
    // ------------------------------------------------------
    void update(sf::RenderWindow& window) {
        if (disabled) return;

        sf::Vector2f world = getMouse(window);
        if (shape.getGlobalBounds().contains(world)) {
            shape.setFillColor(sf::Color(235, 235, 235));
        } else {
            shape.setFillColor(sf::Color(220, 220, 220));
        }
    }

    // ------------------------------------------------------
    // ✔ 檢查有沒有被點擊
    // ------------------------------------------------------
    bool clicked(const sf::Event& e, sf::RenderWindow& window) const {
        if (disabled) return false;

        if (e.type == sf::Event::MouseButtonReleased &&
            e.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2f world = window.mapPixelToCoords(
                { e.mouseButton.x, e.mouseButton.y }
            );
            return shape.getGlobalBounds().contains(world);
        }
        return false;
    }

    // ------------------------------------------------------
    // ✔ 繪製
    // ------------------------------------------------------
    void draw(sf::RenderWindow& window) const {
        window.draw(shape);
        window.draw(text);
    }

    // ------------------------------------------------------
    // ✔ 置中文字（支援中文 UTF-32）
    // ------------------------------------------------------
    void centerText() {
        sf::FloatRect tb = text.getLocalBounds();
        sf::Vector2f pos = shape.getPosition();
        sf::Vector2f origin = shape.getOrigin();

        float cx = pos.x;
        float cy = pos.y;

        if (origin.x == 0.f && origin.y == 0.f) {
            cx = pos.x + shape.getSize().x / 2.f;
            cy = pos.y + shape.getSize().y / 2.f;
        }

        text.setOrigin(tb.left + tb.width / 2.f,
                       tb.top  + tb.height / 2.f);
        text.setPosition(cx, cy);
    }

private:
    // ------------------------------------------------------
    // ✔ 取得滑鼠座標
    // ------------------------------------------------------
    sf::Vector2f getMouse(sf::RenderWindow& window) const {
        sf::Vector2i pixel = sf::Mouse::getPosition(window);
        return window.mapPixelToCoords(pixel);
    }
};
