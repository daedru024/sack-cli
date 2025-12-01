#include "ui/pages/in_room_page.hpp"
#include "ui/common/ui_common.hpp"
#include "ui/common/ui_background.hpp"
#include "ui/widgets/color_selector.hpp"

#include <algorithm>
#include <iostream>

extern std::vector<Room> rooms;
extern int currentRoomIndex;
extern sf::View uiView;
extern void drawBackground(sf::RenderWindow&);
extern bool UI_TEST_MODE;
extern GamePlay gameData;

// ============================================================
// UI 常數
// ============================================================
namespace LobbyUI {
    constexpr float WIDTH         = 800.f;
    constexpr float HEIGHT        = 600.f;

    // 標題
    constexpr float TITLE_X       = 240.f;
    constexpr float TITLE_Y       = 40.f;

    // 玩家列表面板
    constexpr float LIST_X        = 40.f;
    constexpr float LIST_Y        = 150.f;
    constexpr float LIST_W        = 420.f;
    constexpr float ROW_H         = 60.f;
    constexpr float LIST_V_PADDING = 20.f;
    constexpr float LIST_HEADER_H  = 40.f;

    constexpr float PLAYER_NAME_X_OFFSET = 30.f;
    constexpr float READY_STATE_X_OFFSET = 260.f;   // panel 內偏移

    // 右側面板
    constexpr float PANEL_X       = 500.f;
    constexpr float PANEL_W       = 240.f;
    constexpr float PANEL_CENTER  = PANEL_X + PANEL_W / 2.f;

    constexpr float EXIT_X        = PANEL_CENTER;
    constexpr float EXIT_Y        = 40.f;
    constexpr float EXIT_W        = 160.f;
    constexpr float EXIT_H        = 50.f;

    constexpr float COLOR_LABEL_Y = 170.f;
    constexpr float COLOR_PICKER_Y = 250.f;

    constexpr float LOCK_Y        = 330.f;
    constexpr float LOCK_W        = 200.f;
    constexpr float LOCK_H        = 55.f;

    constexpr float READY_Y       = 400.f;
    constexpr float READY_W       = 200.f;
    constexpr float READY_H       = 70.f;

    constexpr float READY_STATE_Y = 500.f;

    constexpr float START_X       = 260.f;
    constexpr float START_Y       = 520.f;
    constexpr float START_W       = 260.f;
    constexpr float START_H       = 70.f;

    constexpr float KICK_HINT_X   = 400.f;
    constexpr float KICK_HINT_Y   = 560.f;

    constexpr float KICK_TIME     = 30.f;
}

static sf::Text mkCenter(
    const sf::Font& font, const std::string& str,
    int size, const sf::Color& col)
{
    sf::Text t(str, font, size);
    t.setFillColor(col);
    auto b = t.getLocalBounds();
    t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    return t;
}

static sf::Text mkLeft(
    const sf::Font& font, const std::string& str,
    int size, const sf::Color& col)
{
    sf::Text t(str, font, size);
    t.setFillColor(col);
    auto b = t.getLocalBounds();
    t.setOrigin(b.left, b.top + b.height / 2.f);
    return t;
}

static float getRowY(int i) {
    using namespace LobbyUI;
    return LIST_HEADER_H + LIST_V_PADDING + i * ROW_H + ROW_H / 2.f;
}

// ============================================================
// 主程式
// ============================================================
void runInRoomPage(
    sf::RenderWindow& window,
    State& state,
    const std::string& username,
    EndReason&)
{
    using namespace LobbyUI;

    sf::Font font;
    loadFontSafe(font);

    int roomIdx = currentRoomIndex;

    if (roomIdx < 0 || roomIdx >= (int)rooms.size()) {
        state = State::RoomInfo;
        return;
    }
    Room& room = gameData.myRoom;

    if (room.n_players == 0) {
        room.resetIfEmpty();
        state = State::RoomInfo;
        return;
    }

    auto& players = room.playerNames;
    int n = room.n_players;
    
    int myIndex = -1;
    for (int i = 0; i < n; i++)
        if (players[i] == username) myIndex = i;

    if (myIndex == -1) {
        state = State::RoomInfo;
        return;
    }

    std::vector<bool> isReady(n, false);
    std::vector<int> colorIndex(room.colors);

    for (int i = 0; i < n; i++)
        isReady[i] = (colorIndex[i] != -1);

    bool isHost = (players[0] == username);

    std::string titleStr = room.name +
        " (" + std::to_string(n) + "/5 Players)";
    if (room.isLocked()) titleStr += " - LOCKED";

    Label title(&font, titleStr, TITLE_X, TITLE_Y, 40,
                sf::Color::White, sf::Color::Black, 5);
    title.centerText();

    float panelHeight = LIST_HEADER_H + 2 * LIST_V_PADDING + n * ROW_H;

    sf::RectangleShape listPanel({LIST_W, panelHeight});
    listPanel.setPosition(LIST_X, LIST_Y);
    listPanel.setFillColor(sf::Color(255,255,255,210));
    listPanel.setOutlineColor(sf::Color(120,120,120));
    listPanel.setOutlineThickness(4);

    Button exitBtn(&font, "Exit",
                   EXIT_X, EXIT_Y, EXIT_W, EXIT_H, true);

    ColorSelector selector(0, 0);
    selector.setLimit(5);
    selector.computePositions(PANEL_CENTER, COLOR_PICKER_Y);

    Label colorLabel(&font, "Choose your color:",
                     PANEL_CENTER, COLOR_LABEL_Y, 24,
                     sf::Color::White, sf::Color::Black, 3);
    colorLabel.centerText();

    Button lockBtn(&font, "LOCK ROOM",
                   PANEL_CENTER, LOCK_Y, LOCK_W, LOCK_H, true);

    Button readyBtn(&font, "READY",
                    PANEL_CENTER, READY_Y, READY_W, READY_H, true);

    Button startBtn(&font, "START GAME",
                    START_X, START_Y, START_W, START_H, true);
    startBtn.text.setCharacterSize(26);

    Label readyStateLabel(&font, "Not Ready",
                          PANEL_CENTER, READY_STATE_Y, 22,
                          sf::Color::White, sf::Color::Black, 2);
    readyStateLabel.centerText();

    // Auto-kick
    sf::Clock idleTimer;
    bool counting = !isReady[myIndex];
    if (counting) idleTimer.restart();

    auto isColorTaken = [&](int c) {
        for (int i = 0; i < n; i++)
            if (i != myIndex && colorIndex[i] == c)
                return true;
        return false;
    };

    // ========================= LOOP ============================
    while (window.isOpen() && state == State::InRoom)
    {
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed)
                window.close();

            if (e.type == sf::Event::Resized) {
                updateBackgroundUI();
            }

            // ===== Exit =====
            if (exitBtn.clicked(e, window)) {
                isReady[myIndex] = false;
                state = State::RoomInfo;
                return;
            }

            // ===== Ready toggle =====
            if (readyBtn.clicked(e, window))
            {
                if (colorIndex[myIndex] == -1) {
                    std::cout << "Choose color first.\n";
                } else {
                    isReady[myIndex] = !isReady[myIndex];

                    if (isReady[myIndex]) {
                        counting = false;
                        gameData.ChooseColor(colorIndex[myIndex]);
                    }
                    else {
                        counting = true;
                        idleTimer.restart();
                    }
                }
            }

            // ===== 顏色選擇 =====
            if (!isReady[myIndex]) {
                selector.updateClick(e, window, [&](int c){ return isColorTaken(c); });
                colorIndex[myIndex] = selector.selected;
            }

            // ===== Host：Lock / Unlock =====
            if (isHost && lockBtn.clicked(e, window))
            {
                int curPlayers = room.n_players;

                if (!room.locked)
                {
                    if (curPlayers < 3)
                    {
                        std::cout << "Need at least 3 players.\n";
                    }
                    else
                    {
                        int err = gameData.LockRoom();
                        if (err == SUCCESS) {
                            // server 廣播會更新
                        }
                        else if (err == NOT_ENOUGH_PLAYERS) {
                            std::cout << "Server: not enough players.\n";
                        }
                    }
                }
                else
                {
                    if (!room.inGame)
                    {
                        int err = gameData.UnlockRoom();
                        if (err == SUCCESS) {
                            // server 會廣播 unlock
                        }
                    }
                }
            }

            // ===== Host：Start Game =====
            if (isHost && startBtn.clicked(e, window))
            {
                bool allReady = true;
                for (bool rdy : isReady)
                    if (!rdy) allReady = false;

                if (room.isLocked() && allReady) {
                    room.inGame = 1;
                    state = State::GameStart;
                    return;
                }
            }
        }

        // ===== Auto-kick（client 端）=====
        if (counting && !isReady[myIndex])
        {
            float elapsed = idleTimer.getElapsedTime().asSeconds();
            if (elapsed > KICK_TIME)
            {
                room.resetIfEmpty();
                state = State::RoomInfo;
                return;
            }
        }

        // Ready 之後，偶爾問一下 server 有沒有 GAMESTART
        if (isReady[myIndex]) {
            if (gameData.GetRoomInfo() == GAME_START) {
                state = State::GameStart;
                return;
            }
        }

        // ====================== Draw =========================
        window.setView(uiView);
        window.clear();
        drawBackground(window);

        title.draw(window);
        window.draw(listPanel);

        {
            sf::Text header = mkCenter(font, "Player List", 24, sf::Color::Black);
            header.setPosition(LIST_X + LIST_W / 2.f, LIST_Y + LIST_HEADER_H / 2.f);
            window.draw(header);
        }

        for (int i = 0; i < n; i++)
        {
            float cy = LIST_Y + getRowY(i);

            std::string nm = players[i];
            if (i == 0) nm += " (Host)";
            if (i == myIndex) nm += " [You]";

            sf::Color nameColor =
                colorIndex[i] >= 0 ? PLAYER_COLORS[colorIndex[i]] : sf::Color::Black;

            sf::Text pname = mkLeft(font, nm, 26, nameColor);
            pname.setPosition(LIST_X + PLAYER_NAME_X_OFFSET, cy);

            float allowedWidth = READY_STATE_X_OFFSET - PLAYER_NAME_X_OFFSET - 20;
            while (pname.getLocalBounds().width > allowedWidth &&
                   pname.getCharacterSize() > 12)
                pname.setCharacterSize(pname.getCharacterSize() - 1);

            window.draw(pname);

            sf::Text r = mkLeft(
                font,
                isReady[i] ? "Ready" : "Not Ready",
                22,
                isReady[i] ? sf::Color(0,150,0) : sf::Color(150,0,0));
            r.setPosition(LIST_X + READY_STATE_X_OFFSET, cy);
            window.draw(r);
        }

        colorLabel.draw(window);
        selector.preview.setFillColor(
            colorIndex[myIndex] >= 0 ?
            PLAYER_COLORS[colorIndex[myIndex]] :
            sf::Color(200,200,200)
        );
        window.draw(selector.preview);
        selector.draw(window, isColorTaken);

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

        if (isHost)
        {
            int curPlayers = room.n_players;

            if (room.locked)
                lockBtn.shape.setFillColor(sf::Color(120,200,120));
            else if (curPlayers < 3)
                lockBtn.shape.setFillColor(sf::Color(180,180,180));
            else
                lockBtn.shape.setFillColor(sf::Color(220,220,220));

            lockBtn.update(window);
            lockBtn.draw(window);

            bool allReady = true;
            for (bool rdy : isReady)
                if (!rdy) allReady = false;

            startBtn.setDisabled(!(room.locked && allReady));

            if (startBtn.disabled)
                startBtn.shape.setFillColor(sf::Color(80,80,80));
            else
                startBtn.shape.setFillColor(sf::Color(120,200,120));

            startBtn.update(window);
            startBtn.draw(window);
        }

        if (!isReady[myIndex] && counting)
        {
            float remain = KICK_TIME - idleTimer.getElapsedTime().asSeconds();
            if (remain < 0) remain = 0;

            sf::Text idleText = mkCenter(
                font, "Auto-kick in: " + std::to_string((int)remain) + "s",
                22, sf::Color::Yellow);
            idleText.setOutlineThickness(2);
            idleText.setOutlineColor(sf::Color::Black);
            idleText.setPosition(KICK_HINT_X, KICK_HINT_Y);
            window.draw(idleText);
        }

        window.display();
    }
}
