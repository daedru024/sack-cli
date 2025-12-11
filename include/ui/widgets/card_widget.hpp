#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "ui/common/ui_common.hpp"
#include "ui/widgets/game_cards.hpp"

class CardWidget {
public:
    int    cardId = -1;
    bool   selected = false;
    bool   isTextureMode = false;

    sf::RectangleShape rect;
    sf::Sprite         sprite;

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

        sprite.setPosition(pos);

        if (id >= 0 && id <= 9) {
            // === 手牌正面：顯示圖片 ===
            isTextureMode = true;
            
            // 1. 設定紋理
            const sf::Texture& tex = GameCardResources::getInstance().getTexture(id);
            sprite.setTexture(tex);

            // 2. 計算縮放 (Fit to size)
            sf::Vector2u texSize = tex.getSize();
            float scaleX = size.x / static_cast<float>(texSize.x);
            float scaleY = size.y / static_cast<float>(texSize.y);
            sprite.setScale(scaleX, scaleY);

            sprite.setColor(sf::Color::White);
            rect.setFillColor(sf::Color::Transparent); // 圖片模式下隱藏背景色
            
        } 
        else {
            // === 牌背/棄牌：顯示色塊 ===
            isTextureMode = false;
            
            // 預設為白色 (外部會再透過 rect.setFillColor 修改顏色)
            rect.setFillColor(sf::Color::White);
        }
    }

    void setFillColor(const sf::Color& color) {
        if (!isTextureMode) {
            rect.setFillColor(color);
        }
    }

    void setPosition(sf::Vector2f pos) {
        rect.setPosition(pos);
        sprite.setPosition(pos);
    }

    void setSize(sf::Vector2f size) {
        rect.setSize(size);
        if (isTextureMode) {
            const sf::Texture* tex = sprite.getTexture();
            if (tex) {
                sf::Vector2u ts = tex->getSize();
                sprite.setScale(size.x / ts.x, size.y / ts.y);
            }
        }
    }

    void setSelected(bool s) {
        selected = s;
        if (isTextureMode) {
            // 圖片模式：變暗
            sprite.setColor(selected ? sf::Color(180, 180, 180) : sf::Color::White);
            // 選取時加紅框
            rect.setOutlineColor(selected ? sf::Color::Red : sf::Color::Black);
            rect.setOutlineThickness(selected ? 5.f : 0.f);
        } else {
            // 色塊模式：加粗藍框
            rect.setOutlineThickness(selected ? 5.f : 3.f);
            rect.setOutlineColor(selected ? sf::Color(0,120,255) : sf::Color::Black);
        }
    }

    bool hitTest(sf::Vector2f p) const {
        return rect.getGlobalBounds().contains(p);
    }

    void draw(sf::RenderWindow& win) const {
        if (isTextureMode) {
            win.draw(sprite);
            // 如果選取中，畫出紅框
            if (selected) win.draw(rect); 
        } else {
            win.draw(rect);
        }
    }

};
