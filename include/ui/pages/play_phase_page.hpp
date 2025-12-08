#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "app/app.hpp"

void runPlayPhasePage(
    sf::RenderWindow& window,
    State&            state,
    EndReason&        reason,
    const std::string& username);
