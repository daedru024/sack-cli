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

    Room();
    Room(int id);

    bool isFull() const;
    std::string hostName() const;
    void resetIfEmpty();
};
