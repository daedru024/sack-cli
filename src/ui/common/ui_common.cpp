#include "ui/common/ui_common.hpp"
#include <algorithm>
#include <sstream>

void loadFontSafe(sf::Font& font)
{
    if (!font.loadFromFile("fonts/SourceHanSansTC-Regular.otf")) {
        font.loadFromFile("fonts/SourceHanSansTC-Regular.otf");
    }
}

// ============================================================
// Button 文字置中（保留你的版本）
// ============================================================
void centerTextInButton(sf::Text &text, const sf::RectangleShape& shape)
{
    sf::FloatRect tb = text.getLocalBounds();
    text.setPosition(
        shape.getPosition().x + (shape.getSize().x - tb.width)/2.f - tb.left,
        shape.getPosition().y + (shape.getSize().y - tb.height)/2.f - tb.top
    );
}

// ============================================================
// 單行置中（UTF-8 → UTF-32 支援中文）
// ============================================================
sf::Text mkCenterText(
    const sf::Font& font,
    const std::string& s,
    int size,
    sf::Color col)
{
    sf::Text t;
    t.setFont(font);
    t.setCharacterSize(size);
    t.setFillColor(col);

    // 中文轉 UTF-32
    t.setString(toUtf32(s));

    auto b = t.getLocalBounds();
    t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);

    return t;
}

// ============================================================
// 多行置中（UTF-8，支援中文）
// ============================================================
sf::Text makeMultilineCenterText(
    const sf::Font& font,
    const std::string& str,
    int size,
    sf::Color col)
{
    sf::Text t;
    t.setFont(font);
    t.setCharacterSize(size);
    t.setFillColor(col);

    // Convert 整段為 UTF-32
    sf::String u32 = toUtf32(str);

    // 設定文字
    t.setString(u32);

    // 計算高度（SFML 不自動算多行）
    float lineSpacing = size * 0.45f; // 行距
    int lines = 1;
    for (auto c : u32) if (c == '\n') lines++;

    float lineHeight = t.getLocalBounds().height + lineSpacing;
    float totalHeight = lineHeight * lines;

    auto b = t.getLocalBounds();
    t.setOrigin(
        b.left + b.width / 2.f,
        b.top + totalHeight / 2.f
    );

    return t;
}
