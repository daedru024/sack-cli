#ifndef __lib_cli_wrap
#define __lib_cli_wrap
#include "libcli.h"
#include <vector>
#include <string>
#include <sstream>

class GamePlay {
    int sockfd, playerID, rem_money;
public:
    GamePlay(const char* servip) : sockfd(Connect(servip)) {}
    int Connect(const char* servip); 
    void GetRoomInfo(std::vector<Room>& rooms);
    int SendBid();
};

class Room {
public:
    bool isPrivate, locked;
    int inGame, id, my_id, n_players;
    std::vector<std::string> playerNames;
    std::vector<int> colors;
    std::string name, password;

    Room(int id) : isPrivate(0), locked(0), inGame(0), 
    id(id), my_id(-1), n_players(0), playerNames(5,""), 
    colors(5, -1), name("Room " + (id+'1')),
    password("10000") {}

    bool isFull() const {
        return (inGame || locked) || n_players == 5;
    }

    std::string hostName() const {
        return playerNames.empty() ? "" : playerNames[0];
    }
};

#endif