#include "ui/common/ui_background.hpp"

sf::Texture& g_bgTex() {
    static sf::Texture tex;
    return tex;
}

sf::Sprite& g_bgSprite() {
    static sf::Sprite sp;
    return sp;
}

sf::RectangleShape& g_bgOverlay() {
    static sf::RectangleShape ov;
    return ov;
}

bool& g_bgLoaded() {
    static bool loaded = false;
    return loaded;
}

void updateBackgroundUI()
{
    float uiW = UI_WIDTH;
    float uiH = UI_HEIGHT;

    float imgW = g_bgTex().getSize().x;
    float imgH = g_bgTex().getSize().y;

    if (imgW == 0 || imgH == 0) return;

    float scale = std::max(uiW / imgW, uiH / imgH);
    g_bgSprite().setScale(scale, scale);

    float posX = (uiW - imgW * scale) / 2.f;
    float posY = (uiH - imgH * scale) / 2.f;
    g_bgSprite().setPosition(posX, posY);

    g_bgOverlay().setSize({uiW, uiH});
}

void initBackground()
{
    if (g_bgLoaded()) return;
    g_bgLoaded() = true;

    g_bgTex().loadFromFile("assets/cat_bg.png");
    g_bgSprite().setTexture(g_bgTex());
    g_bgOverlay().setFillColor(sf::Color(0, 0, 0, 100));

    updateBackgroundUI();
}
