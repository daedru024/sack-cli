#pragma once
#include <string>
#include <vector>

class Room {
public:
    bool isPrivate;
    bool locked;        // 房間是否 locked（host 設定）
    int inGame;         // 是否正在遊戲中 0=否, 1=是
    int id;
    int n_players;

    std::vector<std::string> playerNames;
    std::vector<int> colors;

    std::string name;
    std::string password;

    Room() {};
    Room(int id)
        : isPrivate(false),
          locked(false),
          inGame(0),
          id(id),
          n_players(0),
          playerNames(5, ""),
          colors(5, -1),
          name("Room " + std::to_string(id+1)),
          password("0000")
    {}

    // ✔ 完全修正 — 不要把 locked 或 inGame 當成 isFull
    bool isFull() const {
        return n_players >= 5;
    }

    bool isLocked() const { return locked; }
    bool isPlaying() const { return inGame != 0; }

    std::string hostName() const {
        return playerNames.empty() ? "" : playerNames[0];
    }

    void resetIfEmpty() {
        if (n_players == 0) *this = Room(id);
    }
};
