#ifndef __lib_cli_wrap
#define __lib_cli_wrap

#include "libcli.h"
#include "room.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <bitset>

// ===== UI 要辨識的 return code =====
#define SUCCESS        0
#define ROOM_FULL      1
#define ROOM_LOCKED    2
#define ROOM_PRIVATE   3
#define WRONG_PIN      4
#define ROOM_PLAYING   5

class GamePlay {
    int sockfd = -1;

public:
    std::string servip, UserName;
    int playerID = -1, rem_money = 0, roomID = -1;
    Room myRoom;

    GamePlay() = default;
    GamePlay(const char* ip, const std::string& user);

    // ====== 基本連線 ======
    int  Connect(const char* ip);
    bool isConnected() const { return sockfd > 0; }
    int  getSock() const { return sockfd; }
    int  EndConn();

    // ====== 房間列表 ======
    void GetRoomInfo(std::vector<Room>& rooms);

    // ====== 加房間 ======
    int JoinRoom(int rid);                  // 公開房
    int JoinRoom(int rid, std::string Pwd); // 私房

    // ====== 房主控制 ======
    int MakePrivate(std::string Pwd);       // 設為私人 + PIN
    int UnlockRoom();                       // 設為公開（修正）
    int LockRoom();                         // 若你 server 有 lock

    // ====== 遊戲 ======
    void Play(int c);
    void RecvBid();
    void SendBid(int amount);
    void Wait();
    void ContinuePlay();
};

#endif
