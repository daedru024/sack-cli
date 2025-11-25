#include "libcliwrap.hpp"


/**** CONNECTION ****/
/********************/

// connect to servip
int GamePlay::Connect(const char* servip) {
    return Conn(servip);
}

// end connection
int GamePlay::EndConn() {
    Close(sockfd);
}


/**** ROOMS ****/
/***************/

// continue playing when game ends
void GamePlay::ContinuePlay() {
    EndConn();
    Connect(servip.c_str());
}

// get room info
void GamePlay::GetRoomInfo(std::vector<Room>& rooms) {
    char buf[MAXLINE];
    Recv(sockfd, buf);
    std::stringstream ss(buf);
    std::string tmp;
    int k;
    for(int i=0; i<3; i++) {
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
    }
    ss >> room.locked >> room.password >> playerID;
    
    return;
}

// join room
int GamePlay::JoinRoom(int rid) {
    return JoinRoom(rid, "10000");
}

// join room (private)
int GamePlay::JoinRoom(int rid, std::string Pwd) {
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
    return 0;
}

// lock room
int GamePlay::LockRoom() {
    //TODO
}

// make room private, return 0 if success
int GamePlay::MakePrivate(std::string Pwd) {
    //TODO
}

// make room public, return 0 if success
int GamePlay::MakePublic() {
    //TODO
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

// send unlock message
int GamePlay::UnlockRoom() {
    //TODO
}

void GamePlay::Wait() {
    //TODO
}