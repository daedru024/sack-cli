#include "libcliwrap.h"

int GamePlay::Connect(const char* servip) {
    return Conn(servip);
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
            if(i != k) err_quit("wrong roomid!");
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
            if(i != k) err_quit("wrong roomid!");
            ss >> rooms[i].n_players >> rooms[i].inGame;
            rooms[i].locked = (rooms[i].inGame == 0);
        }
    }
}