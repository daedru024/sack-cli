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
#define NOT_ENOUGH_PLAYERS 6
#define WRONG_DIGIT 7
#define PRIVATE_FAIL 8
#define GAME_START 4096

class GamePlay {
    int sockfd, playerID, rem_money, roomID, color;
    std::string servip, UserName;
    std::bitset<10> MASKUc, MASKSt;
    time_t lst_conn;
public:
    Room myRoom;
    GamePlay() {}
    GamePlay(const char* servip, std::string s) : servip(servip), UserName(s), sockfd(-1) {}
    
    /**** VARIABLES ****/
    /*******************/
    int Color() { return color; }
    int Money() { return rem_money; }
    int PlayerID() { return playerID; }
    int RoomID() { return roomID; }
    int Sockfd() { return sockfd; }
    std::string Username() { return UserName; }

    /**** CONNECTION ****/
    /********************/
    // connect to servip
    int Connect();
    // end connection
    int EndConn();
    // is connected (placeholder, will be improved)
    bool isConnected();
    // Connect/reconnect
    int Reconnect();


    /**** ROOMS ****/
    /***************/
    // continue playing when game ends
    void ContinuePlay();
    // choose color, return 0 if success
    int ChooseColor(int c);
    // get room info
    void GetRoomInfo(std::vector<Room>& rooms);
    // get room info (specific)
    int GetRoomInfo(int rid, Room& room, std::string buf);
    // get room info when in room, catch GAMESTART signal
    int GetRoomInfo();
    // join room
    int JoinRoom(int rid);
    // join room (private)
    int JoinRoom(int rid, std::string Pwd);
    // lock room, return 0 if success
    int LockRoom();
    // make room private, return -1 if player is not host
    int MakePrivate(std::string Pwd);
    // make room public, return -1 if player is not host
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
};

/**** HELPER FUNCTIONS ****/
/**************************/
bool ss_empty(const std::stringstream& ss);

#endif