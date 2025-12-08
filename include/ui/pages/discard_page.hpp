#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "app/app.hpp"

void runDiscardPage(
    sf::RenderWindow& window,
    State&            state,
    EndReason&        reason,
    const std::string& username
);