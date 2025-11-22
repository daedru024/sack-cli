#pragma once
#include <SFML/Graphics.hpp>
#include <string>

#include "ui/common/ui_state.hpp"
#include "room.hpp"
#include "libcliwrap.h"

void runInRoomPage(
    sf::RenderWindow &window,
    State &state,
    const std::string &username,
    EndReason &reason
);
