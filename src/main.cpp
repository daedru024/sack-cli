#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include "button.hpp"
#include "textbox.hpp"
#include "label.hpp"
#include "ui_state.hpp"
#include "color_selector.hpp"
#include "ui_element.hpp"

// ==================== 固定 UI View（800x600） ====================
sf::View uiView(sf::FloatRect(0, 0, UI_WIDTH, UI_HEIGHT));

// ==================== 全域背景：用函式包起來避免初始化順序問題 ====================
// （這個寫法是你說「可以正常」的第一份）
sf::Texture& g_bgTex() {
    static sf::Texture tex;
    return tex;
}
sf::Sprite& g_bgSprite() {
    static sf::Sprite s;
    return s;
}
sf::RectangleShape& g_bgOverlay() {
    static sf::RectangleShape r;
    return r;
}
bool& g_bgLoaded() {
    static bool loaded = false;
    return loaded;
}

// 在「UI 座標空間」(800x600) 裡做等比例縮放
void updateBackgroundUI()
{
    float uiW = UI_WIDTH;
    float uiH = UI_HEIGHT;

    float imgW = g_bgTex().getSize().x;
    float imgH = g_bgTex().getSize().y;

    if (imgW == 0 || imgH == 0) return;

    float scale = std::max(uiW / imgW, uiH / imgH);
    g_bgSprite().setScale(scale, scale);

    float posX = (uiW - imgW * scale) / 2.f;
    float posY = (uiH - imgH * scale) / 2.f;
    g_bgSprite().setPosition(posX, posY);

    g_bgOverlay().setSize({uiW, uiH});
}

void initBackground()
{
    if (g_bgLoaded()) return;
    g_bgLoaded() = true;

    g_bgTex().loadFromFile("assets/cat_bg.png");
    g_bgSprite().setTexture(g_bgTex());
    g_bgOverlay().setFillColor(sf::Color(0, 0, 0, 100));

    updateBackgroundUI();
}

// ==================== 房間資料 ====================

struct Room {
    int id;

    std::vector<std::string> playerNames; // index 0 = Host

    bool isPrivate = false;
    std::string password;

    bool locked = false;      // 房間是否已鎖定人數
    int  lockedPlayers = 0;   // 鎖定時的人數（遊戲人數）

    bool inGame = false;

    std::string name;

    int players() const {
        return static_cast<int>(playerNames.size());
    }

    bool isFull() const {
        if (inGame) return true;
        if (locked) return players() >= lockedPlayers;
        // 未鎖定時最大 5 人
        return players() >= 5;
    }

    std::string hostName() const {
        return playerNames.empty() ? "" : playerNames[0];
    }

    void resetIfEmpty()
    {
        if (!playerNames.empty()) return;
        isPrivate     = false;
        password.clear();
        locked        = false;
        lockedPlayers = 0;
        inGame        = false;
    }
};

// 初始假資料
std::vector<Room> rooms = {
    {1, {"Alice"},        false, "", false, 0, false, "Room 1"},
    {2, {"Bob","Carol"},  true,  "1234", false, 0, false, "Room 2"},
    {3, {},               false, "", false, 0, false, "Room 3"}
};

int currentRoomIndex = -1;

enum class EndReason {
    None,
    RoomsFull,
    UserExit,
    WrongKeyTooMany,
    Timeout
};

// 全域：計算目前 private 房間數（可以忽略某一間，用於設定時排除自己）
int countPrivateRooms(const Room* ignore = nullptr)
{
    int cnt = 0;
    for (auto &r : rooms) {
        if (&r == ignore) continue;
        if (r.isPrivate) cnt++;
    }
    return cnt;
}

// ==================== Username Page ====================

void runUsernamePage(sf::RenderWindow& window, State& state,
                     std::string& username, EndReason& reason)
{
    sf::Font font;
    font.loadFromFile("fonts/NotoSans-Regular.ttf");

    Label title(&font, "Cat In The Sack", 180, 70, 56,
                sf::Color::White, sf::Color::Black, 4);

    sf::RectangleShape panel({500, 300});
    panel.setFillColor(sf::Color(255,255,255,220));
    panel.setOutlineColor(sf::Color(100,100,100));
    panel.setOutlineThickness(4);
    panel.setPosition(150, 150);

    Label enterUser(&font, "Enter Username", 260, 160, 32,
                    sf::Color::White, sf::Color::Black, 3);

    TextBox usernameBox(&font, 250, 230, 300, 55);
    Button okBtn(&font, "START", 310, 330, 180, 65);
    Button exitBtn(&font, "Exit", 650, 20, 120, 40);

    while (window.isOpen() && state == State::UsernameInput)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            usernameBox.handleEvent(event, window);

            if (okBtn.clicked(event, window)) {
                username = usernameBox.buffer;
                if (!username.empty())
                    state = State::RoomInfo;
            }

            if (exitBtn.clicked(event, window)) {
                reason = EndReason::UserExit;
                state  = State::EndConn;
            }
        }

        okBtn.update(window);
        exitBtn.update(window);

        window.setView(uiView);
        window.clear();

        window.draw(g_bgSprite());
        window.draw(g_bgOverlay());

        window.draw(panel);
        title.draw(window);
        enterUser.draw(window);
        usernameBox.draw(window);
        okBtn.draw(window);
        exitBtn.draw(window);

        window.display();
    }
}

// ==================== Host Setting Page ====================
// 只在「進入空房的房主」第一次進來時出現：決定 public / private + 密碼。
// 三間房最多只允許兩間 private。
void runHostSettingPage(sf::RenderWindow& window, State& state,
                        EndReason& /*reason*/, Room& room,
                        const std::string& username)
{
    sf::Font font;
    font.loadFromFile("fonts/NotoSans-Regular.ttf");

    Label title(&font, "Room Privacy", 240, 50, 46,
                sf::Color::White, sf::Color::Black, 4);

    std::string hostStr = "Host: " + username;
    Label hostLabel(&font, hostStr, 240, 110, 24,
                    sf::Color::White, sf::Color::Black, 2);

    Button publicBtn (&font, "Public", 190, 180, 180, 60);
    Button privateBtn(&font, "Private", 410, 180, 180, 60);

    publicBtn.text.setCharacterSize(28);
    privateBtn.text.setCharacterSize(28);
    auto centerText = [](Button& b){
        sf::FloatRect tb = b.text.getLocalBounds();
        b.text.setPosition(
            b.shape.getPosition().x + (b.shape.getSize().x - tb.width)/2.f - tb.left,
            b.shape.getPosition().y + (b.shape.getSize().y - tb.height)/2.f - tb.top
        );
    };
    centerText(publicBtn);
    centerText(privateBtn);

    bool isPrivate = room.isPrivate;     // 預設帶入原本設定
    std::string pwBuf = room.password;   // 若原本是 private，保留原密碼（可改）

    // 密碼（private 時）
    const int PASS_LEN = 4;
    sf::RectangleShape pwBox[PASS_LEN];
    for (int i = 0; i < PASS_LEN; i++) {
        pwBox[i].setSize({50, 60});
        pwBox[i].setOutlineThickness(3);
        pwBox[i].setOutlineColor(sf::Color::Black);
        pwBox[i].setFillColor(sf::Color(255,255,255,230));
    }

    Label pwLabel(&font, "Password (4 digits)", 230, 260, 22,
                  sf::Color::White, sf::Color::Black, 2);

    Button confirmBtn(&font, "CONFIRM", 300, 420, 200, 70);
    centerText(confirmBtn);

    std::string errorMsg;

    while (window.isOpen() && state == State::HostSetting)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            // Public / Private 選擇（只是暫存，最後 Confirm 再檢查限制）
            if (publicBtn.clicked(event, window)) {
                isPrivate = false;
                pwBuf.clear();
                errorMsg.clear();
            }
            if (privateBtn.clicked(event, window)) {
                isPrivate = true;
                errorMsg.clear();
            }

            // 密碼輸入（private 時才吃 TextEntered）
            if (isPrivate && event.type == sf::Event::TextEntered) {
                if (event.text.unicode == 8) {          // Backspace
                    if (!pwBuf.empty()) pwBuf.pop_back();
                } else if (event.text.unicode >= '0' &&
                           event.text.unicode <= '9') { // digit
                    if (pwBuf.size() < PASS_LEN)
                        pwBuf.push_back(static_cast<char>(event.text.unicode));
                }
            }

            // 確認
            if (confirmBtn.clicked(event, window)) {
                // 若選 private，要檢查密碼長度 & private 房數限制
                if (isPrivate) {
                    if (pwBuf.size() != PASS_LEN) {
                        errorMsg = "Password must be 4 digits.";
                        continue;
                    }
                    int privCnt = countPrivateRooms(&room);
                    if (privCnt >= 2) {
                        errorMsg = "At most 2 PRIVATE rooms allowed.";
                        isPrivate = false; // 驗證沒過，回到 public 狀態
                        pwBuf.clear();
                        continue;
                    }
                }
                // 寫入房間設定
                room.isPrivate = isPrivate;
                room.password  = isPrivate ? pwBuf : "";
                state = State::InRoom;
                return;
            }
        }

        // hover 效果
        publicBtn.update(window);
        privateBtn.update(window);
        confirmBtn.update(window);

        window.setView(uiView);
        window.clear();

        window.draw(g_bgSprite());
        window.draw(g_bgOverlay());

        title.draw(window);
        hostLabel.draw(window);

        // Public / Private 顯示
        publicBtn.shape.setFillColor(!isPrivate ? sf::Color(200,240,200)
                                                : sf::Color(220,220,220));
        privateBtn.shape.setFillColor(isPrivate ? sf::Color(200,240,200)
                                                : sf::Color(220,220,220));
        publicBtn.draw(window);
        privateBtn.draw(window);

        // 密碼區（只有 private 時顯示）
        if (isPrivate) {
            pwLabel.draw(window);
            for (int i = 0; i < PASS_LEN; i++) {
                pwBox[i].setPosition(230 + i * 60.f, 300);
                window.draw(pwBox[i]);

                if (i < (int)pwBuf.size()) {
                    sf::Text digit(std::string(1, pwBuf[i]), font, 32);
                    digit.setFillColor(sf::Color::Black);
                    digit.setPosition(240 + i * 60.f, 298);
                    window.draw(digit);
                }
            }
        }

        // 錯誤訊息
        if (!errorMsg.empty()) {
            sf::Text e(errorMsg, font, 20);
            e.setFillColor(sf::Color::Red);
            e.setPosition(230, 380);
            window.draw(e);
        }

        confirmBtn.draw(window);

        window.display();
    }
}

// ==================== RoomInfo Page ====================

void runRoomInfoPage(sf::RenderWindow& window, State& state,
                     EndReason& reason, const std::string& username)
{
    sf::Font font;
    font.loadFromFile("fonts/NotoSans-Regular.ttf");

    Label title(&font, "Select a Room", 200, 40, 50,
                sf::Color::White, sf::Color::Black, 4);

    Button exitBtn(&font, "Exit", 650, 20, 120, 40);

    Button roomBtn[3] = {
        Button(&font, "Room 1", 120, 140, 360, 80),
        Button(&font, "Room 2", 120, 260, 360, 80),
        Button(&font, "Room 3", 120, 380, 360, 80)
    };

    const int PASS_LEN = 4;
    std::string keyInput;
    sf::RectangleShape passBox[PASS_LEN];
    for (int i = 0; i < PASS_LEN; i++) {
        passBox[i].setSize({40, 50});
        passBox[i].setFillColor(sf::Color(255,255,255,230));
        passBox[i].setOutlineColor(sf::Color::Black);
        passBox[i].setOutlineThickness(3);
    }

    Label passLabel(&font, "Password (4 digits)", 520, 205, 22,
                    sf::Color::White, sf::Color::Black, 2);

    Button joinBtn(&font, "JOIN", 520, 360, 150, 60);

    Label selectedLabel(&font, "", 500, 150, 24,
                        sf::Color::White, sf::Color::Black, 3);

    int  selected      = -1;
    bool needsKey      = false;
    std::string errorMsg;
    int  wrongKeyCount = 0;

    sf::Clock timer;

    while (window.isOpen() && state == State::RoomInfo)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            // 離開整個 lobby
            if (exitBtn.clicked(event, window)) {
                reason = EndReason::UserExit;
                state  = State::EndConn;
            }

            auto canClick = [&](int i) {
                Room& r = rooms[i];
                if (r.inGame) return false;
                if (r.isFull()) return false;
                return true;
            };

            // 選房間
            for (int i = 0; i < 3; i++) {
                if (canClick(i) && roomBtn[i].clicked(event, window)) {
                    selected = i;
                    Room& r = rooms[i];
                    needsKey = r.isPrivate && !r.playerNames.empty();
                    keyInput.clear();
                    errorMsg.clear();
                    selectedLabel.text.setString("Selected: " + r.name);
                }
            }

            // 密碼輸入
            if (needsKey && selected != -1 &&
                event.type == sf::Event::TextEntered)
            {
                if (event.text.unicode >= '0' && event.text.unicode <= '9') {
                    if (keyInput.size() < PASS_LEN)
                        keyInput.push_back(static_cast<char>(event.text.unicode));
                }
                if (event.text.unicode == 8 && !keyInput.empty())
                    keyInput.pop_back();
            }

            // JOIN
            if (selected != -1 && joinBtn.clicked(event, window))
            {
                Room& r = rooms[selected];

                if (r.inGame || r.isFull()) {
                    errorMsg = "Room is full.";
                    continue;
                }

                if (r.playerNames.empty()) {
                    // 空房：成為房主，先去 HostSetting 選公開/私密
                    r.playerNames.push_back(username);
                    currentRoomIndex = selected;
                    state = State::HostSetting;
                } else {
                    // 已有玩家：若是 private 需輸入密碼
                    if (r.isPrivate) {
                        if (keyInput != r.password) {
                            wrongKeyCount++;
                            errorMsg = "Wrong password!";
                            keyInput.clear();
                            if (wrongKeyCount >= 3) {
                                reason = EndReason::WrongKeyTooMany;
                                state  = State::EndConn;
                            }
                            continue;
                        }
                    }
                    // 加入房間（避免重複）
                    if (std::find(r.playerNames.begin(),
                                  r.playerNames.end(),
                                  username) == r.playerNames.end())
                    {
                        r.playerNames.push_back(username);
                    }
                    currentRoomIndex = selected;
                    state = State::InRoom;
                }
            }
        }

        // 60 秒 timeout
        float remain = 60.f - timer.getElapsedTime().asSeconds();
        if (remain <= 0.f) {
            reason = EndReason::Timeout;
            state  = State::EndConn;
        }

        // ===== Render =====
        exitBtn.update(window);
        for (int i = 0; i < 3; i++)
            roomBtn[i].update(window);
        joinBtn.update(window);

        window.setView(uiView);
        window.clear();

        window.draw(g_bgSprite());
        window.draw(g_bgOverlay());

        title.draw(window);

        // Timer
        sf::Text timerText("Time left: " + std::to_string((int)remain) + "s",
                           font, 26);
        timerText.setFillColor(sf::Color::White);
        timerText.setOutlineColor(sf::Color::Black);
        timerText.setOutlineThickness(2);
        timerText.setPosition(300, 520);
        window.draw(timerText);

        exitBtn.draw(window);

        // 三間房資訊
        for (int i = 0; i < 3; i++)
        {
            Room&   r = rooms[i];
            Button& b = roomBtn[i];

            bool full = r.isFull();

            sf::Color base(210,230,250);
            if (full)      base = sf::Color(180,180,180);
            if (r.inGame)  base = sf::Color(255,215,180);
            if (selected == i) base = sf::Color(255,240,140);

            b.shape.setFillColor(base);
            b.text.setString("");
            b.draw(window);

            // Host
            std::string hostName = r.hostName();
            sf::Text hostTx(hostName.empty() ? "Host: -" : "Host: " + hostName,
                            font, 20);
            hostTx.setFillColor(sf::Color::Black);
            hostTx.setPosition(b.shape.getPosition().x + 12,
                               b.shape.getPosition().y + 8);
            window.draw(hostTx);

            // Room name
            sf::Text roomName(r.name, font, 28);
            roomName.setFillColor(sf::Color::Black);
            roomName.setPosition(b.shape.getPosition().x + 130,
                                 b.shape.getPosition().y + 25);
            window.draw(roomName);

            // 人數 / Locked 狀態
            sf::Text st("", font, 20);
            st.setFillColor(sf::Color::Black);
            std::string status;
            if (r.playerNames.empty()) {
                status = "Empty room";
            } else {
                status = std::to_string(r.players()) + " players";
                if (r.locked) {
                    status += " (Locked " + std::to_string(r.lockedPlayers) + ")";
                }
            }
            st.setString(status);
            st.setPosition(b.shape.getPosition().x + 12,
                           b.shape.getPosition().y + 55);
            window.draw(st);

            // PRIVATE 標籤
            if (r.isPrivate) {
                sf::RectangleShape tag({80,22});
                tag.setFillColor(sf::Color(255,200,200));
                tag.setOutlineColor(sf::Color(180,30,30));
                tag.setOutlineThickness(2);
                tag.setPosition(b.shape.getPosition().x + b.shape.getSize().x - 90,
                                b.shape.getPosition().y + 28);
                window.draw(tag);

                sf::Text tx("PRIVATE", font, 14);
                tx.setFillColor(sf::Color(180,30,30));
                tx.setPosition(tag.getPosition().x + 8,
                               tag.getPosition().y + 2);
                window.draw(tx);
            }
        }

        // 右側選中 + 密碼區
        if (selected != -1)
        {
            Room& rsel = rooms[selected];

            selectedLabel.draw(window);

            // Host 顯示
            std::string h = rsel.hostName();
            if (!h.empty()) {
                sf::Text hostInfo("Host: " + h, font, 20);
                hostInfo.setFillColor(sf::Color::White);
                hostInfo.setOutlineColor(sf::Color::Black);
                hostInfo.setOutlineThickness(2);
                hostInfo.setPosition(500, 180);
                window.draw(hostInfo);
            }

            // JOIN 按鈕
            joinBtn.draw(window);

            // 密碼 UI：標題在上，四格在下
            if (needsKey) {
                passLabel.draw(window);

                for (int i = 0; i < PASS_LEN; i++) {
                    passBox[i].setPosition(520 + i * 55.f, 240);
                    window.draw(passBox[i]);

                    if (i < (int)keyInput.size()) {
                        sf::Text digit(std::string(1, keyInput[i]), font, 32);
                        digit.setFillColor(sf::Color::Black);
                        digit.setPosition(528 + i * 55.f, 238);
                        window.draw(digit);
                    }
                }
            }

            // 錯誤訊息：在密碼區下方，不擋 JOIN
            if (!errorMsg.empty()) {
                sf::Text e(errorMsg, font, 20);
                e.setFillColor(sf::Color::Red);
                e.setPosition(520, 305);
                window.draw(e);
            }
        }

        window.display();
    }
}

// ==================== InRoom Page ====================

void runInRoomPage(sf::RenderWindow &window, State &state,
                   const std::string &username, EndReason &/*reason*/)
{
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
    int n = players.size();

    // 再檢查一次：房間竟然是空的，就回 RoomInfo
    if (n == 0) {
        room.resetIfEmpty();
        state = State::RoomInfo;
        return;
    }

    int myIndex = 0;
    for (int i = 0; i < n; i++)
        if (players[i] == username) myIndex = i;

    std::vector<bool> isReady(n, false);
    std::vector<int>  colorIndex(n, -1);

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

    // 顏色選擇：位置跟你原本 A 版靠右下一排的設計一致
    ColorSelector selector(0,0);
    selector.setLimit(5);            // 最多 5 種顏色
    selector.computePositions(640, 260);  // 中心在 (640,260)

    Label readyStateLabel(&font, "Not Ready", 540, 500, 22,
                          sf::Color::White, sf::Color::Black, 2);

    // Auto-kick
    sf::Clock idleTimer;
    const float KICK_TIME = 30.f;
    bool counting = true; // 未 ready 時倒數
    idleTimer.restart();  // 一進房就開始倒數

    auto isColorTaken = [&](int col){
        for (int i = 0; i < n; i++)
            if (i != myIndex && colorIndex[i] == col)
                return true;
        return false;
    };

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
                    room.resetIfEmpty();

                state = State::RoomInfo;
                return;
            }

            // ===== Ready toggle（必須先選顏色） =====
            if (readyBtn.clicked(event, window))
            {
                if (colorIndex[myIndex] == -1) {
                    // 沒選顏色不能 ready
                    std::cout << "Ready denied: choose color first.\n";
                } else {
                    bool nowReady = !isReady[myIndex];
                    isReady[myIndex] = nowReady;

                    if (nowReady) {
                        // 按下 READY → 停止倒數 & 不顯示文字
                        counting = false;
                    } else {
                        // 變回 NOT READY → 重新從 30 秒倒數
                        counting = true;
                        idleTimer.restart();
                    }
                }
            }

            // ===== 顏色選擇（只限自己 & 未 ready） =====
            if (!isReady[myIndex]) {
                selector.updateClick(event, window,
                    [&](int c){ return isColorTaken(c); });
                colorIndex[myIndex] = selector.selected;
            }

            // ===== Host：鎖定房間（Lock Room） =====
            if (isHost && lockBtn.clicked(event, window))
            {
                int curPlayers = room.players();

                if (!room.locked) {
                    // 解鎖狀態下，至少 3 人才可以鎖定
                    if (curPlayers >= 3 && curPlayers <= 5) {
                        room.locked        = true;
                        room.lockedPlayers = curPlayers;
                    }
                } else {
                    // 已鎖定 → 可以手動解鎖（遊戲尚未開始）
                    if (!room.inGame) {
                        room.locked        = false;
                        room.lockedPlayers = 0;
                    }
                }
            }

            // ===== Host：Start Game =====
            if (isHost && startBtn.clicked(event, window))
            {
                bool allReady = true;
                for (bool r : isReady)
                    if (!r) allReady = false;

                int curPlayers = room.players();
                // 鎖定 + 全 ready + 人數等於鎖定人數（且 >=3） 才能開始
                bool canStart = room.locked &&
                                allReady &&
                                (curPlayers == room.lockedPlayers) &&
                                (curPlayers >= 3);

                if (canStart) {
                    room.inGame = true;
                    state = State::GameStart; // 之後你可以接 Game 畫面
                    return;
                }
            }
        } // poll events

        // ===== 自動處理鎖定狀態（人數變化） =====
        {
            int curPlayers = room.players();

            // 人數到 5 → 自動鎖定（如果尚未鎖定）
            if (!room.locked && curPlayers == 5) {
                room.locked        = true;
                room.lockedPlayers = 5;
            }

            // 鎖定時若有人離開（人數 < lockedPlayers）→ 自動解鎖
            if (room.locked && curPlayers < room.lockedPlayers) {
                room.locked        = false;
                room.lockedPlayers = 0;
            }
        }

        // ===== Auto-kick：只有 NOT READY 且 counting=true 時才處理 =====
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

        // ===== Draw =====
        window.setView(uiView);
        window.clear();
        window.draw(g_bgSprite());
        window.draw(g_bgOverlay());

        title.draw(window);
        window.draw(listPanel);

        // 玩家列表
        for (int i = 0; i < n; i++)
        {
            std::string nm = players[i];
            if (i == 0) nm += " (Host)";
            if (i == myIndex) nm += " [You]";

            sf::Text pname(nm, font, 26);
            pname.setFillColor(colorIndex[i] >= 0 ?
                               PLAYER_COLORS[colorIndex[i]] :
                               sf::Color::Black);

            float nameX = 70;
            float nameY = 160 + i * 60;
            pname.setPosition(nameX, nameY);

            // 避免名字擋到 Ready
            float maxRight = 230.f;
            float allowedWidth = maxRight - nameX;

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

        // 顏色選擇區
        colorLabel.draw(window);

        selector.preview.setFillColor(
            colorIndex[myIndex] >= 0 ?
            PLAYER_COLORS[colorIndex[myIndex]] :
            sf::Color(200,200,200)
        );
        window.draw(selector.preview);
        selector.draw(window, isColorTaken);

        // 按鈕
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
            // Lock Button 外觀
            int curPlayers = room.players();
            bool canLock = (!room.locked && curPlayers >= 3 && curPlayers <= 5);
            if (room.locked)
                lockBtn.shape.setFillColor(sf::Color(120,200,120));  // 已鎖定
            else if (!canLock)
                lockBtn.shape.setFillColor(sf::Color(80,80,80));      // 不可鎖
            else
                lockBtn.shape.setFillColor(sf::Color(220,220,220));   // 一般

            centerText(lockBtn);
            lockBtn.update(window);
            lockBtn.draw(window);

            // Start Game 按鈕外觀
            bool allReady = true;
            for (bool r : isReady) if (!r) allReady = false;
            int curPlayers2 = room.players();
            bool canStart = room.locked &&
                            allReady &&
                            (curPlayers2 == room.lockedPlayers) &&
                            (curPlayers2 >= 3);

            if (canStart)
                startBtn.shape.setFillColor(sf::Color(120,200,120));
            else
                startBtn.shape.setFillColor(sf::Color(30,30,30)); // 深灰

            centerText(startBtn);
            startBtn.update(window);
            startBtn.draw(window);
        }

        // Idle countdown：只有 NOT READY 且 counting 時顯示
        if (!isReady[myIndex] && counting)
        {
            float remain = KICK_TIME - idleTimer.getElapsedTime().asSeconds();
            if (remain < 0) remain = 0;

            sf::Text idleText("Auto-kick in: " + std::to_string((int)remain) + "s",
                              font, 22);
            idleText.setFillColor(sf::Color::White);
            idleText.setOutlineThickness(2);
            idleText.setOutlineColor(sf::Color::Black);
            idleText.setPosition(320, 560);
            window.draw(idleText);
        }

        window.display();
    }
}

// ==================== End Connection Page ====================

void runEndConnPage(sf::RenderWindow& window, State& state, EndReason reason)
{
    sf::Font font;
    font.loadFromFile("fonts/NotoSans-Regular.ttf");

    std::string msg;
    switch (reason) {
        case EndReason::RoomsFull:
            msg = "All rooms are full.\nConnection closed."; break;
        case EndReason::UserExit:
            msg = "You exited the lobby.\nConnection closed."; break;
        case EndReason::WrongKeyTooMany:
            msg = "Too many wrong passwords.\nConnection closed."; break;
        case EndReason::Timeout:
            msg = "Timeout: You did not join a room in 60s.\nConnection closed."; break;
        default:
            msg = "Connection ended."; break;
    }

    Button okBtn(&font, "OK", 330, 350, 160, 60);

    while (window.isOpen() && state == State::EndConn)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (okBtn.clicked(event, window))
                window.close();
        }

        okBtn.update(window);

        window.setView(uiView);
        window.clear();

        window.draw(g_bgSprite());
        window.draw(g_bgOverlay());

        sf::RectangleShape panel({520, 260});
        panel.setPosition(140, 170);
        panel.setFillColor(sf::Color(255,255,255,240));
        panel.setOutlineColor(sf::Color(180,180,180));
        panel.setOutlineThickness(3);
        window.draw(panel);

        Label title(&font, "Disconnected", 240, 190, 40,
                    sf::Color::White, sf::Color::Black, 4);
        title.draw(window);

        sf::Text tx(msg, font, 24);
        tx.setFillColor(sf::Color::Black);
        tx.setPosition(170, 240);
        window.draw(tx);

        okBtn.draw(window);

        window.display();
    }
}

// ==================== MAIN ====================

int main()
{
    initBackground();

    sf::RenderWindow window(sf::VideoMode(800, 600),
                            "Cat In The Sack",
                            sf::Style::Default);
    window.setVerticalSyncEnabled(true);

    State state = State::UsernameInput;
    EndReason reason = EndReason::None;
    std::string username;

    while (window.isOpen())
    {
        switch (state)
        {
            case State::UsernameInput:
                runUsernamePage(window, state, username, reason);
                break;

            case State::RoomInfo:
                runRoomInfoPage(window, state, reason, username);
                break;

            case State::HostSetting:
                if (currentRoomIndex >= 0 &&
                    currentRoomIndex < static_cast<int>(rooms.size()))
                {
                    runHostSettingPage(window, state, reason,
                                       rooms[currentRoomIndex], username);
                }
                else {
                    state = State::RoomInfo;
                }
                break;

            case State::InRoom:
                runInRoomPage(window, state, username, reason);
                break;

            case State::GameStart:
            {
                window.setView(uiView);
                window.clear();
                window.draw(g_bgSprite());
                window.draw(g_bgOverlay());
                sf::Font font;
                font.loadFromFile("fonts/NotoSans-Regular.ttf");
                sf::Text tx("Game starting ... (placeholder)", font, 28);
                tx.setFillColor(sf::Color::White);
                tx.setOutlineColor(sf::Color::Black);
                tx.setOutlineThickness(2);
                tx.setPosition(120, 260);
                window.draw(tx);
                window.display();

                // 目前先直接回到 RoomInfo，之後你可以接真正的遊戲狀態
                state = State::RoomInfo;
                break;
            }

            case State::ReEstablish:
                // 現在沒用到，你之後接上 server 再實作
                state = State::RoomInfo;
                break;

            case State::EndConn:
                runEndConnPage(window, state, reason);
                break;
        }
    }

    return 0;
}
