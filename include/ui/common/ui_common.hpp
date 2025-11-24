#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

void loadFontSafe(sf::Font& font);
void centerTextInButton(sf::Text& t, const sf::RectangleShape& box);

// UTF-8 → UTF-32（支援中文）
inline sf::String toUtf32(const std::string& s)
{
    return sf::String::fromUtf8(s.begin(), s.end());
}

// 單行置中（支援中文）
sf::Text mkCenterText(
    const sf::Font& font,
    const std::string& s,
    int size,
    sf::Color col
);

// 多行置中（支援中文）
sf::Text makeMultilineCenterText(
    const sf::Font& font,
    const std::string& str,
    int size,
    sf::Color col
);
