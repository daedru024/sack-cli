#pragma once
// =====================
// Standard Libraries
// =====================
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <algorithm>

// =====================
// Game Data
// =====================
#include "room.hpp"
#include "libcliwrap.hpp"

// =====================
// Global UI (800x600 view + background)
// =====================
#include "ui/common/ui_background.hpp"
#include "ui/common/ui_common.hpp"
#include "ui/common/ui_state.hpp"

// =====================
// Pages
// =====================
#include "ui/pages/endconn_page.hpp"
#include "ui/pages/host_setting_page.hpp"
#include "ui/pages/in_room_page.hpp"
#include "ui/pages/room_info_page.hpp"
#include "ui/pages/rules_page.hpp"
#include "ui/pages/username_page.hpp"
#include "ui/pages/starthand_page.hpp"
#include "ui/pages/discard_page.hpp"
#include "ui/pages/play_phase_page.hpp"
#include "ui/pages/settlement_page.hpp"

// =====================
// Widgets
// =====================
#include "ui/widgets/button.hpp"
#include "ui/widgets/color_selector.hpp"
#include "ui/widgets/label.hpp"
#include "ui/widgets/textbox.hpp"
#include "ui/widgets/ui_element.hpp"
#include "ui/widgets/bid_panel.hpp"
#include "ui/widgets/card_widget.hpp"
#include "ui/widgets/hand_panel.hpp"
#include "ui/widgets/game_cards.hpp"

extern std::vector<Room> rooms;
extern GamePlay gameData;
extern int currentRoomIndex;
extern std::string servip;
extern const float UI_WIDTH;
extern const float UI_HEIGHT;
extern bool UI_TEST_MODE;

