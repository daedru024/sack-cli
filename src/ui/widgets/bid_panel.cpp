#include "ui/widgets/bid_panel.hpp"

BidPanel::BidPanel() {}

void BidPanel::init(const sf::Font& font, float startX, float startY) {
    // --- 定義主題配色 (Theme Colors) ---
    sf::Color colText    = sf::Color(101, 67, 33);   // 深棕色文字
    sf::Color colBorder  = sf::Color(101, 67, 33);   // 深棕色邊框
    sf::Color colBg      = sf::Color(255, 248, 220); // 米黃色底 (Cornsilk)
    sf::Color colArrow   = sf::Color(160, 82, 45);   // 赭色箭頭 (Sienna)
    
    sf::Color colOkFill  = sf::Color(144, 238, 144); // 淺綠 (PaleGreen)
    sf::Color colOkLine  = sf::Color(34, 139, 34);   // 森林綠
    
    sf::Color colPassFill= sf::Color(255, 182, 193); // 淺粉紅 (LightPink)
    sf::Color colPassLine= sf::Color(178, 34, 34);   // 火磚紅 (Firebrick)

    // ---------------------------------------------------
    // 1. "Bid" 標題
    // ---------------------------------------------------
    labelTitle = mkCenterText(font, "Bid", 36, colText);
    labelTitle.setOutlineColor(sf::Color(255, 255, 255, 200));
    labelTitle.setOutlineThickness(2.f);
    // 設在最左邊
    sf::FloatRect b = labelTitle.getLocalBounds();
    labelTitle.setOrigin(b.left, b.top + b.height/2.f);
    labelTitle.setPosition(startX, startY);

    float curX = startX + b.width + 30.f; // 增加間距

    // ---------------------------------------------------
    // 2. 減少箭頭
    // ---------------------------------------------------
    arrowDown.setPointCount(3);
    arrowDown.setPoint(0, {0, 0});   // 左上
    arrowDown.setPoint(1, {24, 0});  // 右上
    arrowDown.setPoint(2, {12, 18}); // 下尖點
    
    arrowDown.setFillColor(colArrow);
    arrowDown.setOutlineColor(colBorder);
    arrowDown.setOutlineThickness(2);
    arrowDown.setOrigin(12, 9);
    arrowDown.setPosition(curX + 12, startY);

    curX += 40.f;

    // ---------------------------------------------------
    // 3. 金額框 (置中於箭頭之間)
    // ---------------------------------------------------
    amountBox.setSize({110, 60});
    amountBox.setFillColor(colBg);
    amountBox.setOutlineColor(colBorder);
    amountBox.setOutlineThickness(3);
    amountBox.setOrigin(55, 30);
    amountBox.setPosition(curX + 55, startY);

    amountText.setFont(font);
    amountText.setCharacterSize(36);
    amountText.setFillColor(colText);

    curX += 110.f + 15.f;

    // ---------------------------------------------------
    // 4. 增加箭頭 
    // ---------------------------------------------------
    arrowUp.setPointCount(3);
    arrowUp.setPoint(0, {12, 0});  // 上尖點
    arrowUp.setPoint(1, {24, 18}); // 右下
    arrowUp.setPoint(2, {0, 18});  // 左下
    
    arrowUp.setFillColor(colArrow);
    arrowUp.setOutlineColor(colBorder);
    arrowUp.setOutlineThickness(2);
    arrowUp.setOrigin(12, 9);
    arrowUp.setPosition(curX + 12, startY);

    curX += 60.f;

    // ---------------------------------------------------
    // 5. OK 按鈕 (圓形，綠色系)
    // ---------------------------------------------------
    float radius = 35.f;
    okBtn.setRadius(radius);
    okBtn.setOrigin(radius, radius);
    okBtn.setPosition(curX + radius, startY);
    okBtn.setFillColor(colOkFill);
    okBtn.setOutlineColor(colOkLine);
    okBtn.setOutlineThickness(3);

    okText = mkCenterText(font, "OK", 24, sf::Color(0, 80, 0)); // 深綠色字
    okText.setOutlineThickness(0);
    okText.setPosition(curX + radius, startY);

    curX += radius * 2 + 30.f; // 按鈕間距

    // ---------------------------------------------------
    // 6. 棄標按鈕 (矩形，紅色系)
    // ---------------------------------------------------
    passBtn.setSize({100, 60});
    passBtn.setOrigin(50, 30);
    passBtn.setPosition(curX + 50, startY);
    passBtn.setFillColor(colPassFill);
    passBtn.setOutlineColor(colPassLine);
    passBtn.setOutlineThickness(3);

    passText = mkCenterText(font, "Pass", 26, sf::Color(100, 0, 0)); // 深紅字
    passText.setOutlineThickness(0);
    passText.setPosition(curX + 50, startY);

    updateText();
}

void BidPanel::setRange(int minV, int maxV) {
    if (this->minVal == minV && this->maxVal == maxV) {
        return;
    }

    this->minVal = minV;
    this->maxVal = maxV;

    if (minVal > maxVal) {
        this->value = maxVal; 
    } else {
        this->value = minVal;
    }
    updateText();
}

void BidPanel::setVisible(bool v) {
    visible = v;
}

BidPanel::Action BidPanel::handleEvent(const sf::Event& e, sf::RenderWindow& win) {
    if (!visible || disabled) return Action::None;

    if (e.type == sf::Event::MouseButtonReleased && 
        e.mouseButton.button == sf::Mouse::Left) 
    {
        sf::Vector2f mp = win.mapPixelToCoords({e.mouseButton.x, e.mouseButton.y});

        if (arrowDown.getGlobalBounds().contains(mp)) {
            if (value > minVal) {
                value--;
                updateText();
            }
        }

        else if (arrowUp.getGlobalBounds().contains(mp)) {
            if (value < maxVal) {
                value++;
                updateText();
            }
        }
        
        else if (okBtn.getGlobalBounds().contains(mp)) {
            if (value >= minVal && value <= maxVal) {
                return Action::Submit;
            }
        }
        
        else if (passBtn.getGlobalBounds().contains(mp)) {
            return Action::Pass;
        }
    }
    return Action::None;
}

void BidPanel::draw(sf::RenderWindow& win) {
    if (!visible) return;

    win.draw(labelTitle);
    win.draw(arrowDown);
    win.draw(arrowUp);
    win.draw(amountBox);
    win.draw(amountText);
    
    win.draw(okBtn);
    win.draw(okText);

    win.draw(passBtn);
    win.draw(passText);
}

void BidPanel::updateText() {
    amountText.setString("$" + std::to_string(value));
    
    // 檢查數值是否合法
    bool isValid = (value >= minVal && value <= maxVal);

    sf::Color colText = sf::Color(101, 67, 33); // 深棕
    sf::Color colRed  = sf::Color(200, 0, 0);   // 紅色警告

    if (!isValid) 
        amountText.setFillColor(colRed);
    else 
        amountText.setFillColor(colText);

    // 置中金額文字
    sf::FloatRect b = amountText.getLocalBounds();
    amountText.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
    amountText.setPosition(amountBox.getPosition());

    // 更新 OK 按鈕狀態 (視覺回饋)
    if (isValid) {
        okBtn.setFillColor(sf::Color(144, 238, 144)); // 亮綠
        okBtn.setOutlineColor(sf::Color(34, 139, 34));
    } else {
        okBtn.setFillColor(sf::Color(150, 150, 150)); // 灰色 (Disabled)
        okBtn.setOutlineColor(sf::Color(100, 100, 100));
    }
}