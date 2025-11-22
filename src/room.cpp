#include "room.hpp"

Room::Room()
    : isPrivate(false),
      locked(false),
      inGame(0),
      id(0),
      n_players(0),
      playerNames(5, ""),
      colors(5, -1),
      name("Room 1"),
      password("0000")
{
}

Room::Room(int id)
    : isPrivate(false),
      locked(false),
      inGame(0),
      id(id),
      n_players(0),
      playerNames(5, ""),
      colors(5, -1),
      name("Room "),
      password("0000")
{
    name += std::to_string(id + 1);
}

bool Room::isFull() const {
    return (inGame || locked) || (n_players >= 5);
}

std::string Room::hostName() const {
    return playerNames.empty() ? "" : playerNames[0];
}

void Room::resetIfEmpty() {
    if (n_players == 0) {
        isPrivate = false;
        locked    = false;
        inGame    = 0;

        password = "0000";
        playerNames.assign(5, "");
        colors.assign(5, -1);

        n_players = 0;
    }
}
