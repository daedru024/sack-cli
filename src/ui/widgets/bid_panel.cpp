#include "ui/widgets/bid_panel.hpp"

BidPanel::BidPanel() {}

void BidPanel::init(const sf::Font& font, float startX, float startY) {
    // ---------------------------------------------------
    // 1. "出價" 標籤
    // ---------------------------------------------------
    labelTitle = mkCenterText(font, "Bid", 32, sf::Color::Black);
    // 設在最左邊
    sf::FloatRect b = labelTitle.getLocalBounds();
    labelTitle.setOrigin(b.left, b.top + b.height/2.f);
    labelTitle.setPosition(startX, startY);

    float curX = startX + b.width + 20.f; // 向右位移

    // ---------------------------------------------------
    // 2. 倒三角形 (減少)
    // ---------------------------------------------------
    arrowDown.setPointCount(3);
    arrowDown.setPoint(0, {0, 0});
    arrowDown.setPoint(1, {24, 0});
    arrowDown.setPoint(2, {12, 18}); // 寬24, 高18
    arrowDown.setFillColor(sf::Color(100, 100, 120));
    arrowDown.setOutlineColor(sf::Color::Black);
    arrowDown.setOutlineThickness(2);
    arrowDown.setOrigin(12, 9); // 中心點
    arrowDown.setPosition(curX + 12, startY);

    curX += 40.f; 

    // ---------------------------------------------------
    // 3. 金額框
    // ---------------------------------------------------
    amountBox.setSize({100, 50});
    amountBox.setFillColor(sf::Color(180, 180, 180)); // 灰色底
    amountBox.setOutlineColor(sf::Color(40, 40, 60));
    amountBox.setOutlineThickness(3);
    amountBox.setOrigin(50, 25);
    amountBox.setPosition(curX + 50, startY);

    amountText.setFont(font);
    amountText.setCharacterSize(28);
    amountText.setFillColor(sf::Color::White); // 白字
    // (位置在 updateText() 動態置中)

    curX += 115.f;

    // ---------------------------------------------------
    // 4. 正三角形 (增加)
    // ---------------------------------------------------
    arrowUp.setPointCount(3);
    arrowUp.setPoint(0, {12, 0});
    arrowUp.setPoint(1, {24, 18});
    arrowUp.setPoint(2, {0, 18});
    arrowUp.setFillColor(sf::Color(100, 100, 120));
    arrowUp.setOutlineColor(sf::Color::Black);
    arrowUp.setOutlineThickness(2);
    arrowUp.setOrigin(12, 9);
    arrowUp.setPosition(curX + 12, startY);

    curX += 50.f;

    // ---------------------------------------------------
    // 5. OK 按鈕 (圓形)
    // ---------------------------------------------------
    float radius = 30.f;
    okBtn.setRadius(radius);
    okBtn.setOrigin(radius, radius);
    okBtn.setPosition(curX + radius, startY);
    okBtn.setFillColor(sf::Color(200, 200, 200));
    okBtn.setOutlineColor(sf::Color(40, 40, 60));
    okBtn.setOutlineThickness(3);

    okText = mkCenterText(font, "OK", 22, sf::Color::White);
    okText.setOutlineColor(sf::Color::Black);
    okText.setOutlineThickness(1.f);
    okText.setPosition(curX + radius, startY);

    curX += radius * 2 + 20.f;

    // ---------------------------------------------------
    // 6. 棄標按鈕 (矩形)
    // ---------------------------------------------------
    passBtn.setSize({100, 50});
    passBtn.setOrigin(50, 25);
    passBtn.setPosition(curX + 50, startY);
    passBtn.setFillColor(sf::Color(200, 200, 200));
    passBtn.setOutlineColor(sf::Color(40, 40, 60));
    passBtn.setOutlineThickness(3);

    passText = mkCenterText(font, "Pass", 24, sf::Color::White);
    passText.setOutlineColor(sf::Color::Black);
    passText.setOutlineThickness(1.f);
    passText.setPosition(curX + 50, startY);

    updateText();
}

void BidPanel::setRange(int minV, int maxV) {
    // 如果範圍沒變，就不要重置 value
    if (this->minVal == minV && this->maxVal == maxV) {
        return;
    }

    this->minVal = minV;
    this->maxVal = maxV;

    // 如果錢不夠付最低標，顯示 maxVal (但會是紅字且無法OK)
    // 否則預設顯示 minVal
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

        // 減少 (Down Arrow)
        if (arrowDown.getGlobalBounds().contains(mp)) {
            if (value > minVal) {
                value--;
                updateText();
            }
        }
        // 增加 (Up Arrow)
        else if (arrowUp.getGlobalBounds().contains(mp)) {
            if (value < maxVal) {
                value++;
                updateText();
            }
        }
        // OK
        else if (okBtn.getGlobalBounds().contains(mp)) {
            // 檢查是否合法 (夠錢且大於上家)
            if (value >= minVal && value <= maxVal) {
                return Action::Submit;
            }
        }
        // 棄標
        else if (passBtn.getGlobalBounds().contains(mp)) {
            return Action::Pass;
        }
    }
    return Action::None;
}

void BidPanel::draw(sf::RenderWindow& win) {
    if (!visible) return;

    // Draw title
    win.draw(labelTitle);

    // Arrows
    win.draw(arrowDown);
    win.draw(arrowUp);

    // Amount Box
    win.draw(amountBox);
    win.draw(amountText);

    // Buttons
    // 若錢不夠，將 OK 按鈕變暗
    bool canAfford = (value >= minVal && value <= maxVal);
    okBtn.setFillColor(canAfford ? sf::Color(200, 200, 200) : sf::Color(100, 100, 100));
    
    win.draw(okBtn);
    win.draw(okText);

    // 棄標總是可用
    win.draw(passBtn);
    win.draw(passText);
}

void BidPanel::updateText() {
    amountText.setString("$" + std::to_string(value));
    
    // 數值不合法時顯示紅色
    if (value < minVal || value > maxVal) 
        amountText.setFillColor(sf::Color::Red);
    else 
        amountText.setFillColor(sf::Color::White);

    // 置中
    sf::FloatRect b = amountText.getLocalBounds();
    amountText.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
    amountText.setPosition(amountBox.getPosition());
}