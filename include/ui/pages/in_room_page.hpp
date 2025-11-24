#pragma once
#include "app/app.hpp"
#include "ui/widgets/color_selector.hpp"
#include <algorithm>
#include <iostream>


void runInRoomPage(
    sf::RenderWindow &window,
    State &state,
    const std::string &username,
    EndReason &reason
);
