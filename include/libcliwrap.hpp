#ifndef __lib_cli_wrap
#define __lib_cli_wrap
#include "libcli.h"
#include "room.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <bitset>

#define SUCCESS 0
#define ROOM_FULL 1
#define ROOM_LOCKED 2
#define ROOM_PRIVATE 3
#define WRONG_PIN 4
#define ROOM_PLAYING 5

class GamePlay {
    int sockfd, playerID, rem_money, roomID;
    std::string servip, UserName;
    std::bitset<10> MASKUc, MASKSt;
public:
    Room myRoom;
    GamePlay() {}
    GamePlay(const char* servip, std::string s) : servip(servip), sockfd(Connect(servip)), UserName(s) {}
    
    /**** CONNECTION ****/
    /********************/
    // connect to servip
    int Connect(const char* servip); 
    // is connected (placeholder, will be improved)
    bool isConnected() { return sockfd >= 0; }
    // end connection
    int EndConn();

    /**** ROOMS ****/
    /***************/
    // continue playing when game ends
    void ContinuePlay();
    // get room info
    void GetRoomInfo(std::vector<Room>& rooms);
    // get room info (specific)
    void GetRoomInfo(int rid, Room& room, std::string buf);
    // join room
    int JoinRoom(int rid);
    // join room (private)
    int JoinRoom(int rid, std::string Pwd);
    // lock room
    int LockRoom();
    // make room private, return 0 if success
    int MakePrivate(std::string Pwd);
    // make room public, return 0 if success
    int MakePublic();
    // send unlock message
    int UnlockRoom();

    /**** GAME MECHANISM ****/
    /************************/
    // play card
    void Play(int c);
    // receive bid info
    void RecvBid();
    // bid
    void SendBid(int amount);
    // TODO
    void Wait();
};

#endif