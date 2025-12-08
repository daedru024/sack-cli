#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <algorithm>
#include <functional>
#include "ui/common/ui_common.hpp"

class BidPanel {
public:
    // 狀態變數
    int value = 0;
    int minVal = 0;
    int maxVal = 0;
    bool visible = false;
    bool disabled = false;

    // UI 元件
    sf::Text labelTitle;        // "出價"
    
    sf::ConvexShape arrowDown;  // 倒三角
    sf::ConvexShape arrowUp;    // 正三角
    
    sf::RectangleShape amountBox;
    sf::Text amountText;        // "$XX"
    
    sf::CircleShape okBtn;      // 圓形 OK
    sf::Text okText;
    
    sf::RectangleShape passBtn; // 矩形 棄標
    sf::Text passText;

    // 回傳動作
    enum class Action {
        None,
        Submit,
        Pass
    };

    BidPanel();

    void init(const sf::Font& font, float startX, float startY);

    // 設定範圍： minV = 前一家出價+1, maxV = 玩家現有錢
    void setRange(int minV, int maxV);

    void setVisible(bool v);

    Action handleEvent(const sf::Event& e, sf::RenderWindow& win);

    void draw(sf::RenderWindow& win);

private:
    void updateText();
};