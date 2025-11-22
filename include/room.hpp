#pragma once
#include <string>
#include <vector>

class Room {
public:
    bool isPrivate;
    bool locked;
    int inGame;
    int id;
    int n_players;

    std::vector<std::string> playerNames;
    std::vector<int> colors;

    std::string name;
    std::string password;

    Room() {};
    Room(int id) : isPrivate(false),
        locked(false),
        inGame(0),
        id(id),
        n_players(0),
        playerNames(5, ""),
        colors(5, -1),
        name("Room "),
        password("0000") {name += std::to_string(id + 1);}

    bool isFull() const { return (inGame || locked) || (n_players >= 5); }
    std::string hostName() const { return playerNames.empty() ? "" : playerNames[0]; }
    void resetIfEmpty() { if(n_players == 0) *this = Room(id); }
};
