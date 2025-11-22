#pragma once
#include <SFML/Graphics.hpp>
#include <string>

#include "ui/widgets/label.hpp"
#include "ui/widgets/textbox.hpp"
#include "ui/widgets/button.hpp"
#include "ui/common/ui_state.hpp"
#include "ui/common/ui_common.hpp"
#include "ui/common/ui_background.hpp"
#include "libcliwrap.h"

void runUsernamePage(
    sf::RenderWindow& window,
    State& state,
    std::string& username,
    EndReason& reason
);
