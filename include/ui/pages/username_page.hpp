#pragma once

#include "ui/widgets/label.hpp"
#include "ui/widgets/textbox.hpp"
#include "ui/widgets/button.hpp"
#include "app/app.hpp"

void runUsernamePage(
    sf::RenderWindow& window,
    State& state,
    std::string& username,
    EndReason& reason
);
