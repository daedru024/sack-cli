#pragma once

enum class State {
    UsernameInput,
    RoomInfo,
    InRoom,
    GameStart,
    HostSetting,
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

