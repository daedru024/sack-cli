#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "ui/common/ui_state.hpp"

void runEndConnPage(
    sf::RenderWindow& window,
    State& state,
    EndReason reason
);
