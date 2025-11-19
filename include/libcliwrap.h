#ifndef __lib_cli_wrap
#define __lib_cli_wrap
#include "libcli.h"
#include <vector>
#include <string>
#include <sstream>
#include <bitset>

#define ROOM_FULL 1
#define ROOM_LOCKED 2
#define ROOM_PRIVATE 3
#define WRONG_PIN 4
#define ROOM_PLAYING 5

class Room {
public:
    bool isPrivate, locked;
    int inGame, id, n_players;
    std::vector<std::string> playerNames;
    std::vector<int> colors;
    std::string name, password;

    Room() {}
    Room(int id) : isPrivate(0), locked(0), inGame(0), 
    id(id), n_players(0), playerNames(5,""), 
    colors(5, -1), name("Room "), password("0000") {
        name += ('1'+id);
    }

    bool isFull() const { return (inGame || locked) || n_players == 5; }

    std::string hostName() const { return playerNames.empty() ? "" : playerNames[0]; }
};

class GamePlay {
    int sockfd, playerID, rem_money, roomID;
    std::string UserName;
    std::bitset<10> MASKUc, MASKSt;
public:
    Room myRoom;
    GamePlay() {}
    GamePlay(const char* servip, std::string s) : sockfd(Connect(servip)), UserName(s) {}
    int Connect(const char* servip); 
    int EndConn();
    void GetRoomInfo(std::vector<Room>& rooms);
    int JoinRoom(int rid);
    int JoinRoom(int rid, std::string Pwd);
    int LockRoom();
    int MakePrivate(std::string Pwd);
    void Play(int c);
    void RecvBid();
    void SendBid(int amount);
    int UnlockRoom();
    void Wait();
};

#endif