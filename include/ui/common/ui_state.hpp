#pragma once

enum class State {
    UsernameInput,
    RoomInfo,
    InRoom,
    GameStart,
    Game,
    Discard,
    Play,
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

