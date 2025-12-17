#pragma once
#include <SFML/Graphics.hpp>
#include "app/app.hpp"
#include <string>

// 結算畫面進入點
void runSettlementPage(
    sf::RenderWindow& window,
    State&            state,
    const std::string& username);