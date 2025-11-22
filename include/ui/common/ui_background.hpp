#pragma once
#include "app/app.hpp"
#include "ui/widgets/ui_element.hpp"
#include <algorithm>

// UI 座標：800×600
inline sf::View uiView(sf::FloatRect(0.f, 0.f, UI_WIDTH, UI_HEIGHT));

sf::Texture&        g_bgTex();
sf::Sprite&         g_bgSprite();
sf::RectangleShape& g_bgOverlay();
bool&               g_bgLoaded();

void updateBackgroundUI();

void initBackground();

inline void drawBackground(sf::RenderWindow& window)
{
    window.draw(g_bgSprite());
    window.draw(g_bgOverlay());
}
