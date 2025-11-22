#include "ui/common/ui_common.hpp"
#include "ui/common/ui_background.hpp"

void loadFontSafe(sf::Font& font)
{
    if (!font.loadFromFile("fonts/NotoSans-Regular.ttf")) {
        // 避免 font crash，給一個 fallback
        font.loadFromFile("fonts/NotoSans-Regular.ttf");
    }
}

void centerTextInButton(sf::Text &text, const sf::RectangleShape& shape)
{
    sf::FloatRect tb = text.getLocalBounds();
    text.setPosition(
        shape.getPosition().x + (shape.getSize().x - tb.width)/2.f - tb.left,
        shape.getPosition().y + (shape.getSize().y - tb.height)/2.f - tb.top
    );
}

