#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "ui/common/ui_state.hpp"
#include "room.hpp"
#include "libcliwrap.h"

void runRoomInfoPage(
    sf::RenderWindow& window,
    State& state,
    EndReason& reason,
    const std::string& username
);
