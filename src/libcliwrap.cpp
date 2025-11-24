#include "libcliwrap.hpp"
#include "libcli.h"
#include <iostream>
#include <cstring>

// ---------------------------------------
// Helper: parse room info “ra”, “ru”
// ---------------------------------------
static void parseRoomInfoString(
    const std::string& msg,
    std::vector<Room>& rooms)
{
    std::istringstream iss(msg);
    std::string tag;

    while (iss >> tag) 
    {
        if (tag == "ra") {
            int rid, num;
            iss >> rid >> num;
            if (rid < 0 || rid >= (int)rooms.size()) continue;

            Room& r = rooms[rid];
            r.id = rid;
            r.inGame = 0;
            r.locked = 0;
            r.n_players = num;

            r.playerNames.assign(5, "");
            r.colors.assign(5, -1);

            for (int i = 0; i < num; i++) {
                std::string uname;
                int col;
                iss >> uname >> col;
                r.playerNames[i] = uname;
                r.colors[i]      = col;
            }

            int privFlag; 
            iss >> privFlag;
            r.isPrivate = (privFlag != 0);
        }
        else if (tag == "ru") {
            int rid, num, rnd;
            iss >> rid >> num >> rnd;

            if (rid < 0 || rid >= (int)rooms.size()) continue;

            Room& r = rooms[rid];
            r.id = rid;
            r.n_players = num;
            r.inGame = 1;
            r.locked = 1;
            r.isPrivate = false;
            r.playerNames.assign(5, "");
            r.colors.assign(5, -1);
        }
    }
}

// ---------------------------------------
// GamePlay
// ---------------------------------------
GamePlay::GamePlay(const char* ip, const std::string& user)
    : servip(ip), UserName(user)
{
    Connect(ip);
}

int GamePlay::Connect(const char* ip)
{
    servip = ip;
    sockfd = Conn(ip);      // libcli.h
    return sockfd;
}

int GamePlay::EndConn()
{
    if (sockfd > 0) {
        Close(sockfd);
        sockfd = -1;
    }
    return 0;
}

// ---------------------------------------
void GamePlay::GetRoomInfo(std::vector<Room>& rooms)
{
    if (!isConnected()) return;

    char recvline[MAXLINE];
    int n = Recv(sockfd, recvline);
    if (n <= 0) return;

    parseRoomInfoString(recvline, rooms);
}

// ---------------------------------------
// 加入房間：公開房
// ---------------------------------------
int GamePlay::JoinRoom(int rid)
{
    return JoinRoom(rid, "");
}

// ---------------------------------------
// 加入房間：私人 + PIN
// ---------------------------------------
int GamePlay::JoinRoom(int rid, std::string Pwd)
{
    if (!isConnected()) return -1;

    int pin = 10000; // server 約定：空 PIN = 10000?
    if (!Pwd.empty()) {
        if (Pwd.size() != 4) return WRONG_PIN;
        pin = std::stoi(Pwd);
    }

    // 送出加入請求
    Join(sockfd, rid, UserName.c_str(), pin);

    // 收 server 回覆
    char recvline[MAXLINE];
    int n = Recv(sockfd, recvline);
    if (n <= 0) return -1;

    if (recvline[0] == 'r' && recvline[1] == 'e') {
        int code = -1;
        sscanf(recvline, "re %d", &code);
        return code; // UI 正在使用這些 code
    }

    // 成功 → 更新 myRoom
    std::string msg(recvline);
    std::istringstream iss(msg);
    std::string tag;
    iss >> tag;

    if (tag == "ra") {
        int rrid, num;
        iss >> rrid >> num;
        Room r(rrid);
        r.n_players = num;

        r.playerNames.assign(5, "");
        r.colors.assign(5, -1);

        for (int i = 0; i < num; i++) {
            std::string uname;
            int col;
            iss >> uname >> col;
            r.playerNames[i] = uname;
            r.colors[i] = col;
        }

        int privFlag;
        iss >> privFlag;
        r.isPrivate = (privFlag != 0);
        r.inGame = 0;
        r.locked = (num >= 5);

        myRoom = r;
    }

    return SUCCESS;
}

// ---------------------------------------
// 設成 private（PIN）
// ---------------------------------------
int GamePlay::MakePrivate(std::string Pwd)
{
    if (Pwd.size() != 4) return WRONG_PIN;
    int pin = std::stoi(Pwd);

    return Privt(sockfd, pin); // server 定義
}

// ---------------------------------------
// 設成 public（修正版）
// ---------------------------------------
int GamePlay::UnlockRoom()
{
    // int er = Unpriv(sockfd); // TODO
    // return er;
    return SUCCESS;
}

int GamePlay::LockRoom()
{
    return Lock(sockfd);
}

// 遊戲階段（暫留）
void GamePlay::Play(int c){}
void GamePlay::RecvBid(){}
void GamePlay::SendBid(int amount){}
void GamePlay::Wait(){}
void GamePlay::ContinuePlay(){}
