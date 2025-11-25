#pragma once

enum class State {
    UsernameInput,
    RoomInfo,
    InRoom,
    GameStart,
    HostSetting,
    Rules,
    ReEstablish,
    EndConn
};

enum class EndReason {
    None,
    RoomsFull,
    UserExit,
    WrongKeyTooMany,
    Timeout
};

