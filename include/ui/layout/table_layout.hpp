#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// 回傳：每個 seat 的名字顯示位置（0 = 自己）
inline std::vector<sf::Vector2f> computeSeatPositions(int nPlayers) {
    std::vector<sf::Vector2f> pos(nPlayers);

    const float W = 800.f;
    const float H = 600.f;

    // seat 0: 自己 → 下方中央
    pos[0] = { W / 2.f, H - 80.f };

    if (nPlayers == 3) {
        pos[1] = { 200.f, 100.f }; // 上左
        pos[2] = { 600.f, 100.f }; // 上右
    } else if (nPlayers == 4) {
        pos[1] = { 120.f, 250.f }; // 左中
        pos[2] = { 400.f, 90.f  }; // 上中
        pos[3] = { 680.f, 250.f }; // 右中
    } else if (nPlayers == 5) {
        pos[1] = { 120.f, 250.f }; // 左中
        pos[2] = { 260.f, 90.f  }; // 上左
        pos[3] = { 540.f, 90.f  }; // 上右
        pos[4] = { 680.f, 250.f }; // 右中
    }

    return pos;
}
