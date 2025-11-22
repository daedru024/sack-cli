#pragma once
#include <SFML/Graphics.hpp>
#include <string>

#include "room.hpp"
#include "ui/widgets/label.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/textbox.hpp"
#include "ui/common/ui_state.hpp"
#include "ui/common/ui_common.hpp"
#include "ui/common/ui_background.hpp"

void runHostSettingPage(
    sf::RenderWindow& window,
    State& state,
    EndReason& reason,
    Room& room,
    const std::string& username
);
