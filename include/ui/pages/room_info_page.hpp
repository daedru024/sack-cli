#pragma once
#include "app/app.hpp"
#include <algorithm>
#include <iostream>

void runRoomInfoPage(
    sf::RenderWindow& window,
    State& state,
    EndReason& reason,
    const std::string& username
);
