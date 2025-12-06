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
    bool blindMode = false; //牌背模式

    // 底部區域 (預設 800x600 底部 1/3)
    float areaLeft   = 80.f;
    float areaRight  = 720.f;
    float areaY      = 430.f;   // 卡牌中心的 Y

    void setArea(float left, float right, float centerY) {
        areaLeft  = left;
        areaRight = right;
        areaY     = centerY;
    }

    // 設定手牌，會依 cardValue 排序 (除非是盲選模式，排序沒意義)
    void setHand(const std::vector<int>& ids, const sf::Font& font) {
        handIds = ids;
        if (!blindMode) { // 只有非盲選才需要排序
            std::sort(handIds.begin(), handIds.end(),
                      [](int a, int b) {
                          return cardValue(a) < cardValue(b);
                      });
        }
        rebuildCards(font);
    }

    // 設定是否為盲選模式 (顯示牌背)
    void setBlindMode(bool blind) {
        blindMode = blind;
    }

    int selectedCardId() const {
        if (selectedIdx < 0 || selectedIdx >= (int)handIds.size()) return -1;
        return handIds[selectedIdx];
    }

    int selectedIndex() const {
        return selectedIdx;
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

        float cardWidth  = 50.f;
        float cardHeight = cardWidth * 1.4f;
        // float spacing    = (totalWidth - n * cardWidth) / (n - 1 <= 0 ? 1 : (n - 1));

        // float x = areaLeft;

        float spacing = 10.f;

        // 計算實際內容需要的總寬度
        float contentW = n * cardWidth + (n - 1) * spacing;

        // 若超出範圍 (雖然10張50寬不太可能超)，則自動縮小間距以避免超出邊界
        if (contentW > totalWidth && n > 1) {
            spacing = (totalWidth - n * cardWidth) / (n - 1);
            contentW = totalWidth;
        }

        float startX = areaLeft + (totalWidth - contentW) / 2.f;

        float x = startX;

        for (int id : handIds) {
            int displayId = blindMode ? 999 : id; // 999代表牌背
            CardWidget cw(font, displayId, {x, areaY - cardHeight / 2.f}, {cardWidth, cardHeight});
            cards.push_back(cw);
            x += cardWidth + spacing;
        }
        
        // reset selection
        selectedIdx = -1;
    }
};