#pragma once

enum class State {
    UsernameInput,
    Rules,
    RoomInfo,
    HostSetting,
    InRoom,
    GameStart,
    Discard,
    Game,
    Settlement,
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

