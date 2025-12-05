#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "app/app.hpp"   // for State, EndReason

void runStartHandPage(
    sf::RenderWindow& window,
    State&            state,
    EndReason&        reason,
    const std::string& username);
