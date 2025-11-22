#pragma once

// =====================
// Standard Libraries
// =====================
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <algorithm>

// =====================
// Global UI (800x600 view + background)
// =====================
#include "ui/common/ui_background.hpp"
#include "ui/common/ui_state.hpp"

// =====================
// Pages
// =====================
#include "ui/pages/username_page.hpp"
#include "ui/pages/room_info_page.hpp"
#include "ui/pages/host_setting_page.hpp"
#include "ui/pages/in_room_page.hpp"
#include "ui/pages/endconn_page.hpp"

// =====================
// Game Data
// =====================
#include "room.hpp"
#include "libcliwrap.h"
