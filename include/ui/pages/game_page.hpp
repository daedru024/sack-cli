#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "app/app.hpp"   // for State, EndReason

void runGamePage(
    sf::RenderWindow& window,
    State&            state,
    EndReason&        reason,
    const std::string& username);
