#include "room_utils.hpp"

extern std::vector<Room> rooms;

int countPrivateRooms(const Room* ignore)
{
    int cnt = 0;
    for (auto &r : rooms)
    {
        if (&r == ignore) continue;
        if (r.isPrivate) cnt++;
    }
    return cnt;
}
