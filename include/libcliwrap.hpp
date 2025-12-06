#ifndef __lib_cli_wrap
#define __lib_cli_wrap
#include "libcli.h"
#include "room.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <bitset>
#include <random>

#define SUCCESS 0
#define ROOM_FULL 1
#define ROOM_LOCKED 2
#define ROOM_PRIVATE 3
#define WRONG_PIN 4
#define ROOM_PLAYING 5
#define NOT_ENOUGH_PLAYERS 6
#define WRONG_DIGIT 7
#define PRIVATE_FAIL 8
#define TOO_MANY_PRIVATE 9

#define GOT_RI 512
#define CHOOSE_RABBIT 1024
#define AUTO_PLAYER 2048
#define GAME_START 4096
#define CONN_CLOSED -4096


class GamePlay {
    int sockfd, playerID, rem_money, roomID, color, played, lst_val, lst_bid;
    std::string servip, UserName;
    std::bitset<10> MASKUc, MASKSt;
    time_t lst_conn;
public:
    Room myRoom;
    int removedCardId = -1;
    //int startFlag = 0;
    std::vector<int> MakeUp, CardsPlayed;
    GamePlay() {std::srand(time(NULL));}
    GamePlay(const char* servip, std::string s) : servip(servip), UserName(s), sockfd(-1) {}
    
    /**** VARIABLES ****/
    /*******************/
    int Color() { return color; }
    int Money() { return rem_money; }
    int PlayerID() { return playerID; }
    int RoomID() { return roomID; }
    int Sockfd() { return sockfd; }
    std::string Username() { return UserName; }
    int PlayedThisRound() { return played; }
    int LastBid() { return lst_bid; }
    int PriceNow() { return lst_val; }
    int Round() { return (myRoom.inGame>0) * ((myRoom.inGame-1)/2+1); }
    int RoomStat() { return (myRoom.inGame==19)*3 + (myRoom.inGame>0 && myRoom.inGame<19) * ((myRoom.inGame+1)%2+1); }

    /**** CONNECTION ****/
    /********************/
    // connect to servip
    int Connect();
    // end connection
    int EndConn();
    // is connected
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
    // start game
    int GameStart();
    // send unlock message
    int UnlockRoom();

    // /**** GAME Request ****/
    // /************************/
    // int StartRequest();

    /**** GAME MECHANISM ****/
    /************************/
    // play card
    bool Play(int c);
    // if RecvPlay() or myRoom.inGame==1 && RecvBid().first == PlayNext(), play card
    int PlayNext();
    // receive bid info {NextPlayerID if no err else code, {playerID, amount}}, **playerID can be negative**
    std::pair<int,std::pair<int,int>> RecvBid();
    // choose rabbit
    void Rabbit(int r);
    // receive play card info. Returns who played this round
    int RecvPlay();
    /*** ^ I need to know how you'd update data during gameplay ***/
    bool HasCard(int cid) const { return !MASKUc[cid]; }
    // bid
    void SendBid(int amount);
    //TODO: get score
};

/**** HELPER FUNCTIONS ****/
/**************************/
bool ss_empty(const std::stringstream& ss);


#endif