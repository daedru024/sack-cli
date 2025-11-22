#pragma once
#include "app/app.hpp"
#include "ui/widgets/color_selector.hpp"
#include <algorithm>
#include <iostream>

// ===== Color table =====
extern const std::array<sf::Color, 5> PLAYER_COLORS;

void runInRoomPage(
    sf::RenderWindow &window,
    State &state,
    const std::string &username,
    EndReason &reason
);
