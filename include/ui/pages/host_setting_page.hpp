#pragma once

#include "room.hpp"
#include "ui/widgets/label.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/textbox.hpp"
#include <iostream>
#include "app/app.hpp"

void runHostSettingPage(
    sf::RenderWindow& window,
    State& state,
    EndReason& reason,
    Room& room,
    const std::string& username
);
