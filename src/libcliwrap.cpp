#include "libcliwrap.hpp"

#define DEBUG

/**** CONNECTION ****/
/********************/
// connect to servip
int GamePlay::Connect() { 
    return Conn(servip.c_str());
}

// end connection
int GamePlay::EndConn() { 
    int n = Close(sockfd);
    sockfd = -1;
    return n;
}

// is connected
bool GamePlay::isConnected() { 
    if(sockfd < 0) return 0;
    fd_set write_fds;
    struct timeval tv;

    FD_ZERO(&write_fds);
    FD_SET(sockfd, &write_fds);
    tv.tv_sec = 0;
    tv.tv_usec = 150000;
    int n;

    if((n = select(sockfd + 1, NULL, &write_fds, NULL, &tv)) < 0)
        err_sys("Select");
    
    return n > 0;
}

// Connect/reconnect
int GamePlay::Reconnect() {
    if(sockfd != -1) Close(sockfd);
    sockfd = Connect();
    if(sockfd < 0) return -1;
    return 0;
}

/**** ROOMS ****/
/***************/

// continue playing when game ends
void GamePlay::ContinuePlay() {
    EndConn();
    Connect();
}

// choose color
int GamePlay::ChooseColor(int c) {
#ifdef DEBUG
    printf("Choosing color %d\n", c);
#endif
    color = c;
    //7 {color}
    std::stringstream ss;
    ss << "7 " << c;
    Write(sockfd, ss.str().c_str(), ss.str().length());
    //get broadcasted room info
    char buf[MAXLINE];
    Recv(sockfd, buf);
    lst_conn = time(NULL);
    if(GetRoomInfo(roomID, myRoom, buf) == GAME_START) 
        return GAME_START;
    if(myRoom.colors[playerID] != c) 
        return -1;
    return 0;
}


// get room info
void GamePlay::GetRoomInfo(std::vector<Room>& rooms) {
#ifdef DEBUG
    printf("Getting room info\n");
#endif  
    char buf[MAXLINE];
    Recv(sockfd, buf);
    lst_conn = time(NULL);
    std::stringstream ss(buf);
    std::string tmp;
    int k;
    for(int i=0; i<3; i++) {
        rooms[i] = Room(i);
        //ra {RoomID} {n_Players} {username[:] color[:]} {code}
        //code 1 if need PIN, 0 otherwise
        //color[i] -1 if player i not ready
        ss >> tmp;
        if(tmp == "ra") {
            ss >> k;
            if(i != k) err_quit("GetRoomInfo: wrong roomID (%d %d)",i,k);
            ss >> rooms[i].n_players;
            for(int j=0; j<rooms[i].n_players; j++) {
                ss >> rooms[i].playerNames[j] >> rooms[i].colors[j];
            }
            ss >> rooms[i].isPrivate;
        }
        //ru {RoomID} {n_Players} {rnd}
        //rnd current round, 0 if room locked
        else if(tmp == "ru") {
            ss >> k;
            if(i != k) err_quit("GetRoomInfo: wrong roomID (%d %d)",i,k);
            ss >> rooms[i].n_players >> rooms[i].inGame;
            rooms[i].locked = (rooms[i].inGame == 0);
        }
    }
}

// get room info (specific)
int GamePlay::GetRoomInfo(int rid, Room& room, std::string buf) {
#ifdef DEBUG
    printf("Getting room %d info\n", rid);
#endif
    std::stringstream ss(buf);
    std::string tmp;
    int k;
    //broadcasted room info 
    //in {RoomID} {n_Players} {username[:] color[:]} {locked} {PIN} {playerID}
    ss >> tmp;
    if(tmp != "in") 
        err_quit("GetRoomInfo: recv %s", buf.c_str());
    ss >> k;
    if(rid != k) 
        err_quit("GetRoomInfo: wrong roomID (%d %d)",rid,k);
    ss >> room.n_players;
    for(int j=0; j<room.n_players; j++) {
        ss >> room.playerNames[j] >> room.colors[j];
#ifdef DEBUG
        printf("Player %d: %s Color %d\n", j, room.playerNames[j].c_str(), room.colors[j]);
#endif
    }
    ss >> room.locked >> room.password >> playerID;
#ifdef DEBUG
    printf("PlayerID: %d\n", playerID);
#endif
    room.isPrivate = (room.password != "10000");
    room.password = room.isPrivate ? room.password : "";
    if(!ss_empty(ss)) {
        ss >> tmp;
        if(tmp == "GAMESTART") return GameStart();
    }
    return 0;
}

// get room info when already in room

int GamePlay::GetRoomInfo() {
    char buf[MAXLINE];
    int status = 0;
    
    // ★ 修正 1: 增加迴圈以處理所有已排隊的訊息，避免卡在舊訊息上 (Buffer Drain)
    while (true) {
        // Recv(sockfd, buf) 預期在沒有資料時返回 -2
        if (Recv(sockfd, buf) == -2) {
            status = 0; // 沒有新的訊息，狀態設為 0
            break; 
        }
        
        if (strlen(buf) == 0) return 0; // 連線關閉

        lst_conn = time(NULL);
        std::stringstream ss(buf);
        std::string s;
        ss >> s;

        // 檢查 1: 是否為獨立的 GAMESTART 訊息
        if (s == "GAMESTART") {
            status = GAME_START;
        } 
        // 檢查 2: 是否為房間更新訊息 (可能附帶 GAMESTART)
        else {
            std::string full_msg = buf;
            // 讓 GetRoomInfo(rid, room, buf) 檢查 GAMESTART 是否被附加
            if (GetRoomInfo(roomID, myRoom, full_msg) == GAME_START) {
                status = GAME_START;
            }
        }
        
        if (status == GAME_START) {
            // 已找到 GAMESTART，跳出訊息處理迴圈
            break;
        }
        
        // 如果不是 GAMESTART (status=0)，繼續下一輪迴圈檢查是否有更多訊息
    }
    
    // Keep-alive (現在只有在 Recv 超時/Buffer 清空時才會執行到)
    if (status == 0) {
        time_t currtime = time(NULL);
        if(difftime(currtime, lst_conn) >= 55) {
            Write(sockfd, "  ", 2);
#ifdef DEBUG
            printf("Alive msg\n");
#endif
            lst_conn = currtime;
        }
        return 0; // 沒有 GAMESTART，返回 0 讓 UI 迴圈繼續
    }
    if(strlen(buf) == 0) return 0; //conn closed
    lst_conn = time(NULL);
    std::stringstream ss(buf);
    std::string s;
    ss >> s;
    if(s == "GAMESTART") return GameStart();
    s = buf;
    return GetRoomInfo(roomID, myRoom, s);
}

// int GamePlay::GetRoomInfo() {
//     char buf[MAXLINE];
//     if (Recv(sockfd, buf) == -2) {
//         time_t currtime = time(NULL);
//         if(difftime(currtime, lst_conn) >= 50) {
//             Write(sockfd, "  ", 2);
// #ifdef DEBUG
//             printf("Alive msg\n");
// #endif
//             lst_conn = currtime;
//         }
//         return 0;
//     }
//     if(strlen(buf) == 0) return 0; //conn closed
//     lst_conn = time(NULL);
//     std::stringstream ss(buf);
//     std::string s;
//     ss >> s;
//     if(s == "GAMESTART") {
//         myRoom.inGame = 1;
//         int r = std::rand() % 10;
//         std::stringstream sst;
//         sst << "19 " << r;
//         Write(sockfd, sst.str().c_str(), sst.str().length());
//         played = 0;
//         rem_money = 15;
//         lst_val = -1;
//         MASKUc = 0;
//         MASKSt = 0;
//         return GAME_START;
//     }
//     s = buf;
//     return GetRoomInfo(roomID, myRoom, s);
// }

// join room
int GamePlay::JoinRoom(int rid) {
    return JoinRoom(rid, "10000");
}

// join room (private)

int GamePlay::JoinRoom(int rid, std::string Pwd) {
#ifdef DEBUG
    printf("Joining room %d with PIN %s\n", rid, Pwd.c_str());
#endif
    int PIN = stoi(Pwd);
    Join(sockfd, rid, UserName.c_str(), PIN);
    char buf[MAXLINE];
    Recv(sockfd, buf);
    lst_conn = time(NULL);
    std::stringstream ss(buf);
    std::string rmerr;
    ss >> rmerr;
    if(rmerr == "re") {
        // 0 Full 1 Locked 2 Private 3 WrongPIN 4 Playing
        ss >> rid;
        return rid+1;
    }
    if(rmerr != "in") err_quit("recv: %s",buf);
    myRoom = Room(rid);
    GetRoomInfo(rid, myRoom, buf);
    roomID = rid;
#ifdef DEBUG
    printf("Joined room %d\n", rid);
#endif
    return 0;
}

// lock room
int GamePlay::LockRoom() {
#ifdef DEBUG
    printf("Locking room %d\n", roomID);
#endif
    if(playerID != 0) return -1;
    Lock(sockfd);
    //get broadcasted room info
    char buf[MAXLINE];
    Recv(sockfd, buf);
    lst_conn = time(NULL);
    GetRoomInfo(roomID, myRoom, buf);
    if(myRoom.locked == false) {
        if(myRoom.n_players < 3)
            return NOT_ENOUGH_PLAYERS;
        return -1;
    }
    return 0;
}

// make room private, return 0 if success
int GamePlay::MakePrivate(std::string Pwd) {
#ifdef DEBUG
    printf("Making room %d private with PIN %s\n", roomID, Pwd.c_str());
#endif
    if(playerID != 0) return -1;
    if(Pwd.size() != 4 && Pwd != "10000") return WRONG_DIGIT;
    int PIN = stoi(Pwd);
    Privt(sockfd, PIN);
    //get broadcasted room info
    char buf[MAXLINE];
    Recv(sockfd, buf);
    if(buf[0] == 'r' && buf[1] == 'e') {
        int code = buf[3] - '0'; // modified
        if (code == 6) return TOO_MANY_PRIVATE; // modified
        return PRIVATE_FAIL;
    }
    lst_conn = time(NULL);
    GetRoomInfo(roomID, myRoom, buf);
    if(Pwd != "10000" && myRoom.password != Pwd) 
        return PRIVATE_FAIL;
    return 0;
}

// make room public, return 0 if success
int GamePlay::MakePublic() {
    if(myRoom.isPrivate == false) return 0;
    return MakePrivate("10000");
}

// start game
int GamePlay::GameStart() {
    myRoom.inGame = 1;
    played = 0;
    rem_money = 15;
    lst_val = -1;
    lst_bid = 0;
    MASKUc = 0;
    MASKSt = 0;
    lst_conn = time(NULL);
    CardsPlayed = std::vector<int>(myRoom.n_players, -1);
    MakeUp = std::vector<int>(myRoom.n_players);
    switch(myRoom.n_players) {
    case 3:
        MakeUp = {3, 6, 0};
        break;
    case 4:
        MakeUp = {2, 4, 6, 0};
        break;
    case 5:
        MakeUp = {2, 3, 4, 6, 0};
    }
    if(playerID == 0) return CHOOSE_RABBIT;
    return GAME_START;
}

// send unlock message
int GamePlay::UnlockRoom() {
#ifdef DEBUG
    printf("Unlocking room %d\n", roomID);
#endif
    if(playerID != 0) return -1;
    Write(sockfd, "2", 1);
    //get broadcasted room info
    char buf[MAXLINE];
    Recv(sockfd, buf);
    lst_conn = time(NULL);
    GetRoomInfo(roomID, myRoom, buf);
    if(myRoom.locked == true) 
        return -1;
    return 0;
}

/**** GAME MECHANISM ****/
/************************/

// play card
bool GamePlay::Play(int c) {
    if(MASKUc[c]) return 0;
    int tmp = (int)MASKUc.to_ulong();
    if(PlayCard(sockfd, playerID, c, tmp) == -1) return 0;
    MASKUc[c] = 1;
    lst_val = c;
    return 1;
}

// if RecvPlay()/RecvBid().first == PlayNext(), play
int GamePlay::PlayNext() { return (playerID+myRoom.n_players-1)%myRoom.n_players; }

void GamePlay::Rabbit(int r) {
    std::stringstream sst;
    sst << "19 " << r;
    Write(sockfd, sst.str().c_str(), sst.str().length());
}

int GamePlay::RecvPlay() {
    //TODO
    //ri {card}
    //c {PlayerID} {code}
    //ap {id}
    char buf[MAXLINE];
    time_t tm = time(NULL);
    if(Recv(sockfd, buf) == -2) {
        if(difftime(tm, lst_conn) >= 55) {
            Write(sockfd, "  ", 2);
            lst_conn = tm;
        }
        return -2;
    }
    lst_conn = tm;
    std::stringstream ss(buf);
    std::string tmp;
    int pID, cd;
    ss >> tmp >> pID;
    if(tmp == "ap") {
        //autoplay or end connection
        //TODO
        return AUTO_PLAYER;
    }
    else if(tmp == "ri") {
        if(MASKUc.to_ulong() != 0) return -1;
        MASKUc[pID] = 1;
        if(playerID == 0) return PlayNext();
        else return CHOOSE_RABBIT;
    }
    if(!ss_empty(ss)) ss >> cd;
    //not pID's round, ignore
    if(cd == 0 || cd == -1) {
        if(pID == playerID) {
            if(lst_val == -1) return -1;
            MASKUc[lst_val] = 0;
            lst_val = -1;
        }
        return -1;
    }
    if(cd == 1) {
        if(++played == myRoom.n_players) {
            //start bidding
            myRoom.inGame++;
            played = 0;
            lst_val = 0;
            lst_bid = 0;
            CardsPlayed = std::vector<int>(myRoom.n_players, -1);
        }
    }
    return pID;
}

// receive bid info
std::pair<int,std::pair<int,int>> GamePlay::RecvBid() {
    //b {PlayerID} {amount} {NextPlayerID} {cardID if amount==0 else -1}
    //be {PlayerID} {amount} {sPlayer}
    //ap {PlayerID}
    char buf[MAXLINE];
    time_t tm = time(NULL);
    if(Recv(sockfd, buf) == -2) {
        if(difftime(tm, lst_conn) >= 55) {
            Write(sockfd, "  ", 2);
            lst_conn = tm;
        }
        return {-2,{-2,-2}};
    }
    lst_conn = tm;
    std::stringstream ss(buf);
    std::string tmp;
    int pID;
    ss >> tmp >> pID;
    if(tmp == "ap") {
        //auto player
        //TODO
        return {AUTO_PLAYER, {-1,-1}};
    }
    int amount, npID;
    if(tmp == "b") {
        ss >> amount >> npID;
        if(npID>=0) {
            if(amount>0) lst_val = amount;
            else if(amount == 0) {
                //abandoned bid
                ss >> CardsPlayed[played];
                if(pID == playerID) {
                    rem_money += MakeUp[played];
                    lst_bid = 0;
                }
                played++;
                return {npID, {pID, amount}};
            }
            if(pID == playerID) lst_bid = amount;
        }
        return {npID, {pID, amount}};
    }
    if(tmp == "be") {
        ss >> amount >> npID >> CardsPlayed[myRoom.n_players-1];
        std::pair<int,std::pair<int,int>> ret;
        if(amount <= 0) ret = {npID, {-1, -1}};
        else {
            if(pID == playerID) {
                rem_money -= amount;
                MASKSt[Round()-1] = 1;
            }
            ret = {npID, {pID, amount}};
        }
        myRoom.inGame++;
        lst_bid = 0;
        lst_val = -1;
        played = 0;
        return ret;
    }
    return {0,{0,0}};
}

// bid
void GamePlay::SendBid(int amount) {
    if(amount>rem_money || amount<lst_val) return;
    Bid(sockfd, playerID, amount, rem_money);
}

bool ss_empty(const std::stringstream& ss) { 
    return(ss.rdbuf()->in_avail() == 0); 
}

// // modified
// int GamePlay::StartRequest() {
// #ifdef DEBUG
//     printf("Sending start request\n");
// #endif

//     // 讓 server 讀到此封包後觸發條件檢查
//     Write(sockfd, "  ", 2);

//     return 0;
// }
