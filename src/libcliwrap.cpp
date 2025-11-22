#include "libcliwrap.h"

int GamePlay::Connect(const char* servip) {
    return Conn(servip);
}

void GamePlay::ContinuePlay() {
    EndConn();
    Connect(servip.c_str());
}

int GamePlay::EndConn() {
    Close(sockfd);
}

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

int GamePlay::JoinRoom(int rid) {
    return JoinRoom(rid, "10000");
}

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
    //broadcasted room info 
    //in {RoomID} {n_Players} {username[:] color[:]} {locked} {PIN}
    if(rmerr != "in") err_quit("recv: %s",buf);
    roomID = rid;
    myRoom = Room(rid);
    ss >> rid;
    if(rid != roomID) err_quit("JoinRoom: wrong roomID (%d %d)",roomID,rid);
    ss >> myRoom.n_players;
    playerID = myRoom.n_players-1;
    for(int i=0; i<=playerID; i++) {
        ss >> myRoom.playerNames[i] >> myRoom.colors[i];
    }
    if(myRoom.playerNames[playerID] != UserName) err_quit("JoinRoom: UserName doesn't match");
    ss >> myRoom.locked;
    ss >> myRoom.password;
    return 0;
}

int GamePlay::LockRoom() {
    //TODO
}

int GamePlay::MakePrivate(std::string Pwd) {
    //TODO
}

void GamePlay::Play(int c) {
    //TODO
}

void GamePlay::RecvBid() {
    //TODO
}

void GamePlay::SendBid(int amount) {
    if(amount>rem_money) return;
    Bid(sockfd, playerID, amount, rem_money);
}

int GamePlay::UnlockRoom() {
    //TODO
}

void GamePlay::Wait() {
    //TODO
}