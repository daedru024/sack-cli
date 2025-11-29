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

// is connected (placeholder, will be improved)
bool GamePlay::isConnected() { 
    return sockfd >= 0; 
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
    GetRoomInfo(roomID, myRoom, buf);
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
void GamePlay::GetRoomInfo(int rid, Room& room, std::string buf) {
#ifdef DEBUG
    printf("Getting room %d info\n", rid);
#endif
    std::stringstream ss(buf);
    std::string tmp;
    int k;
    //broadcasted room info 
    //in {RoomID} {n_Players} {username[:] color[:]} {locked} {PIN} {playerID}
    ss >> tmp;
    if(tmp != "in") //TODO
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
    return;
}

// get room info when already in room
int GamePlay::GetRoomInfo() {
    char buf[MAXLINE];
    if (Recv(sockfd, buf) == -2) return 0;
    if(strlen(buf) == 0) return 0;
    std::stringstream ss(buf);
    std::string s;
    ss >> s;
    if(s == "GAMESTART") return GAME_START;
    s = buf;
    GetRoomInfo(roomID, myRoom, s);
    return 0;
}

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
        // failed
        return PRIVATE_FAIL;
    }
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
    GetRoomInfo(roomID, myRoom, buf);
    if(myRoom.locked == true) 
        return -1;
    return 0;
}

/**** GAME MECHANISM ****/
/************************/

// play card
void GamePlay::Play(int c) {
    //TODO
}

// receive bid info
void GamePlay::RecvBid() {
    //TODO
}

// bid
void GamePlay::SendBid(int amount) {
    if(amount>rem_money) return;
    Bid(sockfd, playerID, amount, rem_money);
}

// wait for GAMESTART signal
// void GamePlay::Wait() {
//     std::string buf = "";
//     while(buf != "GAMESTART") {
//         //
//     }
// }