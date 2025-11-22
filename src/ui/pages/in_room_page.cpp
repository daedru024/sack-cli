#include "ui/pages/in_room_page.hpp"
#include "ui/common/ui_background.hpp"
#include "ui/common/ui_common.hpp"

#include <algorithm>
#include <iostream>

// ===== Color table =====
extern const std::array<sf::Color, 5> PLAYER_COLORS;

extern std::vector<Room> rooms;
extern int currentRoomIndex;


void runInRoomPage(
    sf::RenderWindow &window,
    State &state,
    const std::string &username,
    EndReason &/*reason*/
){
    sf::Font font;
    font.loadFromFile("fonts/NotoSans-Regular.ttf");

    int roomIdx = currentRoomIndex;

    // 保險：避免 roomIdx 無效導致 segfault
    if (roomIdx < 0 || roomIdx >= (int)rooms.size()) {
        state = State::RoomInfo;
        return;
    }

    Room &room = rooms[roomIdx];

    auto &players = room.playerNames;
    int n = room.n_players;

    // 再檢查一次：房間竟然是空的，就回 RoomInfo
    if (n == 0) {
        room = Room(room.id);
        state = State::RoomInfo;
        return;
    }

    int myIndex = 0;
    for (int i = 0; i < n; i++)
        if (players[i] == username) myIndex = i;

    std::vector<bool> isReady(n, false);
    std::vector<int>  colorIndex(room.colors);

    for(int i=0; i<n; i++)
        isReady[i] = (colorIndex[i] != -1);

    bool isHost = (!players.empty() && players[0] == username);

    Label title(&font, "Room Lobby", 240, 40, 52,
                sf::Color::White, sf::Color::Black, 5);

    float panelHeight = 80 + n * 60;
    sf::RectangleShape listPanel({420, panelHeight});
    listPanel.setPosition(40, 150);
    listPanel.setFillColor(sf::Color(255,255,255,210));
    listPanel.setOutlineColor(sf::Color(120,120,120));
    listPanel.setOutlineThickness(4);

    Button exitBtn(&font, "Exit", 650, 20, 120, 40);

    Button lockBtn(&font, "LOCK ROOM", 500, 330, 200, 55);
    Button readyBtn(&font, "READY", 500, 400, 200, 70);
    Button startBtn(&font, "START GAME", 40, 480, 260, 70);
    startBtn.text.setCharacterSize(26);

    auto centerText = [](Button& b){
        sf::FloatRect tb = b.text.getLocalBounds();
        b.text.setPosition(
            b.shape.getPosition().x + (b.shape.getSize().x - tb.width)/2.f - tb.left,
            b.shape.getPosition().y + (b.shape.getSize().y - tb.height)/2.f - tb.top
        );
    };
    centerText(lockBtn);
    centerText(readyBtn);
    centerText(startBtn);

    Label colorLabel(&font, "Choose your color:", 500, 170, 24,
                     sf::Color::White, sf::Color::Black, 3);

    // 顏色選擇
    ColorSelector selector(0,0);
    selector.setLimit(5);
    selector.computePositions(640, 260);

    Label readyStateLabel(&font, "Not Ready", 540, 500, 22,
                          sf::Color::White, sf::Color::Black, 2);

    // Auto-kick
    sf::Clock idleTimer;
    const float KICK_TIME = 30.f;
    bool counting = true;  // main.cpp：一進房間開始倒數
    idleTimer.restart();

    auto isColorTaken = [&](int c){
        for (int i = 0; i < n; i++)
            if (i != myIndex && colorIndex[i] == c)
                return true;
        return false;
    };

    // ============================ LOOP =============================
    while (window.isOpen() && state == State::InRoom)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            // ===== Exit =====
            if (exitBtn.clicked(event, window))
            {
                auto it = std::find(players.begin(), players.end(), username);
                if (it != players.end()) players.erase(it);

                if (players.empty())
                    room = Room(room.id);

                state = State::RoomInfo;
                return;
            }

            // ===== Ready toggle（必須先選顏色） =====
            if (readyBtn.clicked(event, window))
            {
                if (colorIndex[myIndex] == -1) {
                    std::cout << "Ready denied: choose color first.\n";
                } else {
                    bool nowReady = !isReady[myIndex];
                    isReady[myIndex] = nowReady;

                    if (nowReady) counting = false;
                    else {
                        counting = true;
                        idleTimer.restart();
                    }
                }
            }

            // ===== 顏色選擇（未 ready 才能選）=====
            if (!isReady[myIndex]) {
                selector.updateClick(event, window,
                    [&](int c){ return isColorTaken(c); });
                colorIndex[myIndex] = selector.selected;
            }

            // ===== Host：鎖定房間 =====
            if (isHost && lockBtn.clicked(event, window))
            {
                int curPlayers = room.n_players;

                if (!room.locked) {
                    if (curPlayers >= 3 && curPlayers <= 5)
                        room.locked = 1;
                } else {
                    if (!room.inGame)
                        room.locked = 0;
                }
            }

            // ===== Host：Start Game =====
            if (isHost && startBtn.clicked(event, window))
            {
                bool allReady = true;
                for (bool r : isReady)
                    if (!r) allReady = false;

                if (room.locked && allReady) {
                    room.inGame = true;
                    state = State::GameStart;
                    return;
                }
            }
        }

        // ===== Auto-lock / unlock =====
        {
            int curPlayers = room.n_players;

            if (!room.locked && curPlayers == 5)
                room.locked = 1;

            // main.cpp 註解掉 lockedPlayers，不實作
        }

        // ===== Auto-kick =====
        if (counting)
        {
            float elapsed = idleTimer.getElapsedTime().asSeconds();
            if (elapsed > KICK_TIME)
            {
                auto it = std::find(players.begin(), players.end(), username);
                if (it != players.end()) players.erase(it);

                state = State::RoomInfo;
                return;
            }
        }

        // ==================== DRAWING =======================
        window.setView(uiView);
        window.clear();

        drawBackground(window);

        title.draw(window);
        window.draw(listPanel);

        // 玩家列表
        for (int i = 0; i < n; i++)
        {
            std::string nm = players[i];
            if (i == 0) nm += " (Host)";
            if (i == myIndex) nm += " [You]";

            sf::Text pname(nm, font, 26);
            pname.setFillColor(
                colorIndex[i] >= 0 ?
                PLAYER_COLORS[colorIndex[i]] :
                sf::Color::Black
            );

            // 防止名稱太長擋到 Ready
            float nameX = 70;
            float allowedWidth = 230.f - nameX;
            pname.setPosition(nameX, 160 + i * 60);

            while (pname.getLocalBounds().width > allowedWidth &&
                   pname.getCharacterSize() > 12)
            {
                pname.setCharacterSize(pname.getCharacterSize() - 1);
            }

            window.draw(pname);

            sf::Text r(isReady[i] ? "Ready" : "Not Ready", font, 24);
            r.setFillColor(isReady[i] ? sf::Color(0,150,0)
                                      : sf::Color(150,0,0));
            r.setPosition(250, 165 + i * 60);
            window.draw(r);
        }

        // 顏色 UI
        colorLabel.draw(window);
        selector.preview.setFillColor(
            colorIndex[myIndex] >= 0 ?
            PLAYER_COLORS[colorIndex[myIndex]] :
            sf::Color(200,200,200)
        );
        window.draw(selector.preview);
        selector.draw(window, isColorTaken);

        // Buttons
        exitBtn.update(window);
        exitBtn.draw(window);

        readyBtn.shape.setFillColor(
            isReady[myIndex] ?
            sf::Color(170,220,170) :
            sf::Color(220,220,220)
        );
        readyBtn.update(window);
        readyBtn.draw(window);

        readyStateLabel.text.setString(
            isReady[myIndex] ? "You are READY" : "Not Ready"
        );
        readyStateLabel.draw(window);

        // Host UI
        if (isHost)
        {
            int curPlayers = room.n_players;
            bool canLock = (!room.locked && curPlayers >= 3 && curPlayers <= 5);

            if (room.locked)
                lockBtn.shape.setFillColor(sf::Color(120,200,120));
            else if (!canLock)
                lockBtn.shape.setFillColor(sf::Color(80,80,80));
            else
                lockBtn.shape.setFillColor(sf::Color(220,220,220));

            centerText(lockBtn);
            lockBtn.update(window);
            lockBtn.draw(window);

            bool allReady = true;
            for (bool r : isReady)
                if (!r) allReady = false;

            if (room.locked && allReady)
                startBtn.shape.setFillColor(sf::Color(120,200,120));
            else
                startBtn.shape.setFillColor(sf::Color(30,30,30));

            centerText(startBtn);
            startBtn.update(window);
            startBtn.draw(window);
        }

        // Auto-kick countdown
        if (!isReady[myIndex] && counting) {
            float remain = KICK_TIME - idleTimer.getElapsedTime().asSeconds();
            if (remain < 0) remain = 0;

            sf::Text idleText(
                "Auto-kick in: " + std::to_string((int)remain) + "s",
                font, 22
            );
            idleText.setFillColor(sf::Color::White);
            idleText.setOutlineThickness(2);
            idleText.setOutlineColor(sf::Color::Black);
            idleText.setPosition(320, 560);
            window.draw(idleText);
        }

        window.display();
    }
}
