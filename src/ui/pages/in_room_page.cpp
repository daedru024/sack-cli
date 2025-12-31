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
    constexpr float TITLE_X       = 260.f;
    constexpr float TITLE_Y       = 40.f;

    // 玩家列表面板
    constexpr float LIST_X        = 40.f;
    constexpr float LIST_Y        = 100.f;
    constexpr float LIST_W        = 420.f;
    constexpr float ROW_H         = 60.f;
    constexpr float LIST_V_PADDING = 20.f;
    constexpr float LIST_HEADER_H  = 40.f;

    constexpr float PLAYER_NAME_X_OFFSET = 30.f;
    constexpr float READY_STATE_X_OFFSET = 260.f;

    // 右側面板
    constexpr float PANEL_X       = 500.f;
    constexpr float PANEL_W       = 240.f;
    constexpr float PANEL_CENTER  = PANEL_X + PANEL_W / 2.f;

    constexpr float EXIT_X        = 660.f;
    constexpr float EXIT_Y        = 40.f;
    constexpr float EXIT_W        = 160.f;
    constexpr float EXIT_H        = 50.f;

    constexpr float COLOR_LABEL_Y = 170.f;
    constexpr float COLOR_PICKER_Y = 250.f;

    constexpr float START_X       = 230.f;
    constexpr float START_Y       = 521.f;
    constexpr float START_W       = 260.f;
    constexpr float START_H       = 70.f;

    constexpr float READY_Y       = 400.f;
    constexpr float READY_W       = 200.f;
    constexpr float READY_H       = 70.f;

    constexpr float READY_STATE_Y = 500.f;

    constexpr float KICK_HINT_X   = 400.f;
    constexpr float KICK_HINT_Y   = 570.f;

    constexpr float KICK_TIME     = 30.f;
}

// ============================================================
// tool text creators
// ============================================================
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

    int myIndex = gameData.PlayerID();
    if (myIndex < 0 || myIndex >= n) {
        gameData.GetRoomInfo();
        myIndex = gameData.PlayerID();
        if (myIndex < 0 || myIndex >= n) {
            state = State::RoomInfo;
            return;
        }
    }

    // READY 狀態：拆 server/local
    std::vector<int>  colorIndex(room.colors);
    std::vector<bool> serverReady(n, false);
    std::vector<bool> localReady(n, false);

    for (int i = 0; i < n; i++)
        if (localReady[i])
            serverReady[i] = true;
    localReady[myIndex] = localReady[myIndex];

    bool isHost = (myIndex == 0);

    std::string titleStr = room.name +
        " (" + std::to_string(n) + "/5 Players)";
    //if (room.isLocked()) titleStr += " - LOCKED";
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

    //  Locked & Start
    Button lockBtn(&font, "LOCK & START",
               START_X, START_Y, START_W, START_H, true);
    lockBtn.text.setCharacterSize(28);
    lockBtn.centerText();


    Button readyBtn(&font, "READY",
                    PANEL_CENTER, READY_Y, READY_W, READY_H, true);

    Label readyStateLabel(&font, "Not Ready",
                          PANEL_CENTER, READY_STATE_Y, 22,
                          sf::Color::White, sf::Color::Black, 2);
    readyStateLabel.centerText();

    // Auto-kick
    sf::Clock idleTimer;
    bool counting = !localReady[myIndex];
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

        // ------------ 同步 server 狀態 ------------
        int prevMyIndex = myIndex;

        int status = gameData.GetRoomInfo();

        if (status == CONN_CLOSED) {
            state = State::RoomInfo; // 或者 State::EndConn
            return;
        }

        
        if (status == GAME_START) {
            state = State::GameStart;
            return;
        }
        // added
        else if(status == CHOOSE_RABBIT) {
            state = State::Discard;
            return;
        }


        room = gameData.myRoom;
        n    = room.n_players;

        if (n == 0) {
            room.resetIfEmpty();
            state = State::RoomInfo;
            return;
        }

        // 更新 playerID
        myIndex = gameData.PlayerID();
        if (myIndex < 0 || myIndex >= n) {
            state = State::RoomInfo;
            return;
        }

        bool prevReady = false;
        if (prevMyIndex >= 0 && prevMyIndex < (int)localReady.size())
            prevReady = localReady[prevMyIndex];
        if (myIndex != prevMyIndex) {
            if (prevMyIndex >= 0 && prevMyIndex < (int)localReady.size())
                localReady[prevMyIndex] = false;
            localReady[myIndex] = prevReady;//serverReady[myIndex];
        }

        // 更新 Ready 狀態
        // 不覆蓋自己正在選的顏色
        for (int i = 0; i < n; i++) {
            if (i == myIndex && !localReady[myIndex]) {
                // 選色中 → 保留 local 的 selector 顏色
                continue;
            }
            colorIndex[i] = room.colors[i];
        }

        serverReady.assign(n, false);
        for (int i = 0; i < n; i++)
            serverReady[i] = (i == myIndex) ? localReady[i] : (colorIndex[i] != -1);

        if ((int)localReady.size() != n) {
            std::vector<bool> newLocal(n, false);
            int copyCnt = std::min((int)localReady.size(), n);
            for (int i = 0; i < copyCnt; i++)
                newLocal[i] = localReady[i];
            localReady.swap(newLocal);
        }

        isHost = (myIndex == 0);

        const_cast<std::vector<std::string>&>(players) = room.playerNames;

        // ========================= event =========================
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed)
                window.close();

            if (e.type == sf::Event::Resized)
                updateBackgroundUI();

            // EXIT
            if (exitBtn.clicked(e, window)) {
                localReady[myIndex] = false;
                state = State::RoomInfo;
                return;
            }

            // READY
            if (readyBtn.clicked(e, window))
            {
                if (colorIndex[myIndex] == -1) {
                    std::cout << "Choose color first.\n";
                }
                else if (!localReady[myIndex])
                {
                    localReady[myIndex] = true;
                    counting = false;

                    gameData.ChooseColor(colorIndex[myIndex]);

                    readyBtn.setDisabled(true);
                }
            }

            // Color select
            if (!localReady[myIndex]) {
                selector.updateClick(e, window,
                    [&](int c){ return isColorTaken(c); });
                colorIndex[myIndex] = selector.selected;
            }

            // ✨ Host: LOCK & START
            if (isHost && lockBtn.clicked(e, window))
            {
                if (room.n_players < 3) {
                    std::cout << "[Host] Need >= 3 players.\n";
                    continue;
                }

                bool allReady = true;
                for (int i = 0; i < n; i++)
                    if (!serverReady[i]) allReady = false;

                if (!allReady) {
                    std::cout << "[Host] Everyone must choose color first.\n";
                    continue;
                }

                // added : 如果回傳 choose_rabbit 就不要再等一次loop
                // gameData.LockRoom();
                int res = gameData.LockRoom();
                std::cout << "[Host] LOCK & START sent.\n";

                if(res == CHOOSE_RABBIT){
                    state = State::Discard;
                    return;
                }

                // 保險起見，雖然應該不會用到

                if(res == GAME_START){
                    state = State::GameStart;
                    return;
                }
            }
        }

        // auto-kick
        if (counting && !localReady[myIndex])
        {
            float elapsed = idleTimer.getElapsedTime().asSeconds();
            if (elapsed > KICK_TIME) {
                room.resetIfEmpty();
                state = State::RoomInfo;
                return;
            }
        }

        // ====================== draw =========================
        window.setView(uiView);
        window.clear();
        drawBackground(window);

        // 標題
        std::string titleStrNow = room.name +
            " (" + std::to_string(n) + "/5 Players)";
        if (room.isLocked()) titleStrNow += " - LOCKED";

        title.text.setString(titleStrNow);
        title.centerText();
        title.draw(window);

        // player list panel
        listPanel.setSize({LIST_W, LIST_HEADER_H + n * ROW_H + LIST_V_PADDING * 2});
        window.draw(listPanel);

        {
            sf::Text header = mkCenter(font, "Player List", 24, sf::Color::Black);
            header.setPosition(LIST_X + LIST_W / 2.f, LIST_Y + LIST_HEADER_H / 2.f);
            window.draw(header);
        }

        // players
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

            bool showReady = serverReady[i];
            sf::Text r = mkLeft(
                font,
                showReady ? "Ready" : "Not Ready",
                22,
                showReady ? sf::Color(0,150,0) : sf::Color(150,0,0)
            );
            r.setPosition(LIST_X + READY_STATE_X_OFFSET, cy);
            window.draw(r);
        }

        // UI elements
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

        // READY button
        if (localReady[myIndex]) {
            readyBtn.setDisabled(true);
            readyBtn.shape.setFillColor(sf::Color(150,200,150));
        }
        else {
            readyBtn.setDisabled(false);
            readyBtn.shape.setFillColor(sf::Color(220,220,220));
        }

        readyBtn.update(window);
        readyBtn.draw(window);

        readyStateLabel.text.setString(
            localReady[myIndex] ? "You are READY" : "Not Ready"
        );
        readyStateLabel.draw(window);

        // ✨ Host 按鈕：LOCK & START
        if (isHost)
        {
            bool canStart = (room.n_players >= 3);
            for (int i = 0; i < n; i++)
                if (!serverReady[i]) canStart = false;

            lockBtn.setDisabled(!canStart);
            lockBtn.shape.setFillColor(
                canStart ?
                    sf::Color(120,200,120) :
                    sf::Color(180,180,180)
            );

            lockBtn.update(window);
            lockBtn.draw(window);
        }

        // auto kick info
        if (!localReady[myIndex] && counting)
        {
            float remain = KICK_TIME - idleTimer.getElapsedTime().asSeconds();
            if (remain < 0) remain = 0;

            sf::Text idleText = mkCenter(
                font, std::to_string((int)remain) + " seconds left",
                26, sf::Color::White);
            idleText.setOutlineThickness(2);
            idleText.setOutlineColor(sf::Color::Black);
            idleText.setPosition(KICK_HINT_X, KICK_HINT_Y);
            idleText.setFillColor(remain <= 10 ? sf::Color::Red : sf::Color::White);
            window.draw(idleText);
        }

        window.display();
    }
}
