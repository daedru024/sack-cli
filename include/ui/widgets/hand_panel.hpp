#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>
#include "ui/widgets/card_widget.hpp"
#include "ui/game/game_cards.hpp"

class HandPanel {
public:
    std::vector<int>         handIds;     // cardId (0..9)
    std::vector<CardWidget>  cards;       // 對應視覺物件
    int                      selectedIdx = -1;

    // 底部區域 (預設 800x600 底部 1/3)
    float areaLeft   = 80.f;
    float areaRight  = 720.f;
    float areaY      = 430.f;   // 卡牌中心的 Y

    void setArea(float left, float right, float centerY) {
        areaLeft  = left;
        areaRight = right;
        areaY     = centerY;
    }

    // 設定手牌，會依 cardValue 排序
    void setHand(const std::vector<int>& ids, const sf::Font& font) {
        handIds = ids;
        std::sort(handIds.begin(), handIds.end(),
                  [](int a, int b) {
                      return cardValue(a) < cardValue(b);
                  });
        rebuildCards(font);
    }

    int selectedCardId() const {
        if (selectedIdx < 0 || selectedIdx >= (int)handIds.size()) return -1;
        return handIds[selectedIdx];
    }

    void clearSelection() {
        if (selectedIdx != -1) {
            cards[selectedIdx].setSelected(false);
        }
        selectedIdx = -1;
    }

    void handleClick(const sf::Event& e, sf::RenderWindow& win) {
        if (e.type != sf::Event::MouseButtonReleased ||
            e.mouseButton.button != sf::Mouse::Left)
            return;

        sf::Vector2f mp((float)e.mouseButton.x, (float)e.mouseButton.y);
        mp = win.mapPixelToCoords((sf::Vector2i)mp);

        int newSel = -1;
        for (int i = 0; i < (int)cards.size(); ++i) {
            if (cards[i].hitTest(mp)) {
                newSel = i;
                break;
            }
        }

        if(newSel == -1) return;

        if (newSel == selectedIdx){
            cards[selectedIdx].setSelected(false);
            selectedIdx = -1;
        }
        else{
            if (selectedIdx != -1)
                cards[selectedIdx].setSelected(false);
            selectedIdx = newSel;
            cards[selectedIdx].setSelected(true);
        }

    }

    void draw(sf::RenderWindow& win) const {
        for (auto& c : cards) c.draw(win);
    }

private:
    void rebuildCards(const sf::Font& font) {
        cards.clear();
        if (handIds.empty()) return;

        float totalWidth = areaRight - areaLeft;
        int   n          = (int)handIds.size();

        float cardWidth  = std::min(70.f, totalWidth / (n + 0.5f));
        float cardHeight = cardWidth * 1.4f;
        float spacing    = (totalWidth - n * cardWidth) / (n - 1 <= 0 ? 1 : (n - 1));

        float x = areaLeft;
        for (int id : handIds) {
            CardWidget cw(font, id, {x, areaY - cardHeight / 2.f}, {cardWidth, cardHeight});
            cards.push_back(cw);
            x += cardWidth + spacing;
        }

        // reset selection
        selectedIdx = -1;
    }
};
