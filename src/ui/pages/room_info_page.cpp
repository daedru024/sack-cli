#include "ui/pages/room_info_page.hpp"
#include "libcliwrap.hpp"
#include "ui/common/ui_common.hpp"
#include "ui/common/ui_background.hpp"

#include <sstream>
#include <algorithm>

extern std::vector<Room> rooms;
extern GamePlay gameData;
extern int currentRoomIndex;
extern sf::View uiView;
extern void drawBackground(sf::RenderWindow&);
extern bool UI_TEST_MODE;

namespace RInfo {
    constexpr float WIDTH        = 800.f;
    constexpr float HEIGHT       = 600.f;

    constexpr float LIST_LEFT_X  = 60.f;
    constexpr float LIST_TOP_Y   = 110.f;
    constexpr float ROOM_BTN_W   = 400.f;
    constexpr float ROOM_BTN_H   = 80.f;
    constexpr float ROOM_BTN_GAP = 30.f;

    constexpr float PANEL_X      = LIST_LEFT_X + ROOM_BTN_W + 30.f;
    constexpr float PANEL_Y      = LIST_TOP_Y;
    constexpr float PANEL_W      = 240.f;
    constexpr float PANEL_H      = 340.f;

    constexpr float TITLE_Y      = 50.f;

    constexpr int  PASS_LEN      = 4;
    constexpr float PASS_W       = 40.f;
    constexpr float PASS_H       = 50.f;
    constexpr float PASS_GAP     = 12.f;

    constexpr float SELECTED_Y   = PANEL_Y + 20.f;
    constexpr float PASS_LABEL_Y = PANEL_Y + 110.f;
    constexpr float PASS_BOX_Y   = PANEL_Y + 150.f;
    constexpr float ERROR_Y      = PANEL_Y + 220.f;
    constexpr float JOIN_BTN_Y   = PANEL_Y + 280.f;

    constexpr float EXIT_W       = 120.f;
    constexpr float EXIT_H       = 40.f;

    constexpr float TIMER_Y      = 520.f;
}

static float btnCenterY(int idx) {
    using namespace RInfo;
    return LIST_TOP_Y + ROOM_BTN_H/2.f + idx * (ROOM_BTN_H + ROOM_BTN_GAP);
}

static float passBoxX(int idx) {
    using namespace RInfo;
    float total = PASS_LEN * PASS_W + (PASS_LEN - 1) * PASS_GAP;
    float start = PANEL_X + (PANEL_W - total) / 2.f;
    return start + idx * (PASS_W + PASS_GAP);
}

void runRoomInfoPage(
    sf::RenderWindow& window,
    State& state,
    EndReason& reason,
    const std::string& username
){
    using namespace RInfo;

    sf::Font font;
    loadFontSafe(font);

    // ---- 初始連線 + 拿房間資訊（用原本 blocking 版，確保一開始有資料）----
    if(gameData.Reconnect() < 0) {
        state = State::EndConn;
        return;
    }
    gameData.GetRoomInfo(rooms);
    if ((int)rooms.size() < 3) rooms.resize(3);

    int selected = -1;
    bool needsKey = false;
    std::string keyInput;
    std::string errorMsg;
    int wrongKeyCount = 0;

    sf::Clock timer;

    auto canClickRoom = [&](int i) {
        Room& r = rooms[i];
        if (r.isPlaying()) return false;
        if (r.isFull())    return false;
        if (r.isLocked())  return false;
        return true;
    };

    auto tryJoin = [&](int idx) -> bool {
        Room& r = rooms[idx];
        errorMsg.clear();

        if (r.isPrivate && (int)keyInput.size() < PASS_LEN) {
            errorMsg = "Please enter 4-digit password.";
            return false;
        }

        int er = (r.isPrivate ?
                 gameData.JoinRoom(idx, keyInput) :
                 gameData.JoinRoom(idx));

        switch (er) {
            case SUCCESS:
                currentRoomIndex = idx;
                state = (gameData.myRoom.n_players == 1 ?
                         State::HostSetting :
                         State::InRoom);
                return true;

            case ROOM_FULL:     errorMsg = "Room full."; break;
            case ROOM_LOCKED:   errorMsg = "Room locked."; break;
            case ROOM_PRIVATE:
                rooms[idx].isPrivate = true;
                needsKey = true;
                errorMsg = "Please enter password.";
                keyInput.clear();
                wrongKeyCount = 0;
                break;

            case WRONG_PIN:
                wrongKeyCount++;
                errorMsg = "Wrong password (" + std::to_string(wrongKeyCount) + "/3)";
                keyInput.clear();
                if (wrongKeyCount >= 3) {
                    reason = EndReason::WrongKeyTooMany;
                    state = State::EndConn;
                    return true;
                }
                break;

            case ROOM_PLAYING:  errorMsg = "Room playing."; break;
            default:
                errorMsg = "Unknown error: " + std::to_string(er);
                break;
        }
        return false;
    };

    // ===== UI Components =====
    Label title(&font, "Select a Room",
                LIST_LEFT_X + ROOM_BTN_W/2.f,
                TITLE_Y, 50,
                sf::Color::White, sf::Color::Black, 4);
    title.centerText();

    Button exitBtn(&font, "Exit",
                   WIDTH - EXIT_W/2.f - 20.f,
                   20.f + EXIT_H/2.f,
                   EXIT_W, EXIT_H, true);

    Button roomBtn[3] = {
        Button(&font, "", LIST_LEFT_X + ROOM_BTN_W/2.f, btnCenterY(0), ROOM_BTN_W, ROOM_BTN_H, true),
        Button(&font, "", LIST_LEFT_X + ROOM_BTN_W/2.f, btnCenterY(1), ROOM_BTN_W, ROOM_BTN_H, true),
        Button(&font, "", LIST_LEFT_X + ROOM_BTN_W/2.f, btnCenterY(2), ROOM_BTN_W, ROOM_BTN_H, true)
    };

    sf::RectangleShape rightPanel({PANEL_W, PANEL_H});
    rightPanel.setPosition(PANEL_X, PANEL_Y);
    rightPanel.setFillColor(sf::Color(255,255,255,220));
    rightPanel.setOutlineColor(sf::Color(120,120,120));
    rightPanel.setOutlineThickness(3.f);

    Label selectedLabel(&font, "", PANEL_X + PANEL_W/2.f,
                        SELECTED_Y, 24, sf::Color::Black);
    selectedLabel.centerText();

    Label passLabel(&font, "Password (4 digits)",
                    PANEL_X + PANEL_W/2.f,
                    PASS_LABEL_Y, 22, sf::Color::Black);
    passLabel.centerText();

    Label errorLabel(&font, "",
                     PANEL_X + PANEL_W/2.f,
                     ERROR_Y, 20, sf::Color(220,50,50));
    errorLabel.centerText();

    Button joinBtn(&font, "JOIN",
                   PANEL_X + PANEL_W/2.f, JOIN_BTN_Y,
                   180.f, 60.f, true);

    sf::RectangleShape passBox[PASS_LEN];
    for (int i=0;i<PASS_LEN;i++){
        passBox[i].setSize({PASS_W, PASS_H});
        passBox[i].setFillColor(sf::Color(255,255,255,230));
        passBox[i].setOutlineColor(sf::Color::Black);
        passBox[i].setOutlineThickness(3);
    }

    // =================================================================
    //                        MAIN LOOP
    // =================================================================
    while (window.isOpen() && state == State::RoomInfo)
    {
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed)
                window.close();

            if (e.type == sf::Event::Resized)
                updateBackgroundUI();

            if (exitBtn.clicked(e, window)) {
                reason = EndReason::UserExit;
                state = State::EndConn;
                return;
            }

            // ---- 點選房間 ----
            for (int i = 0; i < 3; i++) {
                if (canClickRoom(i) && roomBtn[i].clicked(e, window)) {
                    selected = i;
                    keyInput.clear();
                    errorMsg.clear();
                    wrongKeyCount = 0;
                    needsKey = rooms[i].isPrivate;

                    selectedLabel.set("Selected: " + rooms[i].name);
                    selectedLabel.centerText();
                }
            }

            // ---- 密碼輸入 ----
            if (needsKey && selected != -1)
            {
                if (e.type == sf::Event::TextEntered)
                {
                    if (e.text.unicode >= '0' && e.text.unicode <= '9')
                    {
                        if ((int)keyInput.size() < PASS_LEN)
                            keyInput.push_back((char)e.text.unicode);
                    }
                    else if (e.text.unicode == 8) { // backspace
                        if (!keyInput.empty())
                            keyInput.pop_back();
                    }
                }

                if (e.type == sf::Event::KeyPressed &&
                    e.key.code == sf::Keyboard::Enter &&
                    (int)keyInput.size() == PASS_LEN)
                {
                    if (tryJoin(selected)) return;
                }

                if (!errorMsg.empty())
                {
                    bool isEnterKey = (e.type == sf::Event::KeyPressed && 
                                    e.key.code == sf::Keyboard::Enter);
                                    
                    bool isEnterChar = (e.type == sf::Event::TextEntered && 
                                    (e.text.unicode == 13 || e.text.unicode == 10));

                    // 只有當事件是「文字輸入」或「按鍵按下」，且「不是 Enter」時，才清除訊息
                    if ((e.type == sf::Event::TextEntered || e.type == sf::Event::KeyPressed) && 
                        !isEnterKey && !isEnterChar)
                    {
                        errorMsg.clear();
                    }
                }
            }

            // ---- JOIN BUTTON ----
            if (selected != -1 && joinBtn.clicked(e, window)) {
                if (tryJoin(selected)) return;
            }
        }

        // ---- Timeout ----
        float remain = 60.f - timer.getElapsedTime().asSeconds();
        if (remain <= 0.f) {
            reason = EndReason::Timeout;
            state = State::EndConn;
            return;
        }

        // ---- 更新按鈕 ----
        exitBtn.update(window);
        bool joinEnabled = (selected != -1) &&
                           (!needsKey || (int)keyInput.size() == PASS_LEN);
        joinBtn.setDisabled(!joinEnabled);
        if (joinEnabled) joinBtn.update(window);

        // ============================================================
        // Draw UI
        // ============================================================
        window.setView(uiView);
        window.clear();
        drawBackground(window);

        title.draw(window);
        exitBtn.draw(window);

        // ---- Timer ----
        std::ostringstream oss;
        oss << (int)remain << " seconds left";
        sf::Text timerTx = mkCenterText(font, oss.str(), 26, sf::Color::White);
        timerTx.setOutlineColor(sf::Color::Black);
        timerTx.setOutlineThickness(2.f);
        timerTx.setPosition(LIST_LEFT_X + ROOM_BTN_W / 2.f, TIMER_Y);
        timerTx.setFillColor(remain <= 10 ? sf::Color::Red : sf::Color::White);
        window.draw(timerTx);

        // ---- 房間清單 ----
        for (int i = 0; i < 3; i++)
        {
            Room& r = rooms[i];
            Button& b = roomBtn[i];

            bool clickable = canClickRoom(i);

            sf::Color base = sf::Color(210,230,250);

            if (r.isLocked() && !r.isPlaying())
                base = sf::Color(180,180,180,200);           // Locked → 灰色、不能點
            else if (!clickable)
                base = r.isPlaying() ? sf::Color(200,150,150,200)
                                     : sf::Color(150,150,150,200);

            if (selected == i)
                base = sf::Color(255,240,140);

            b.setDisabled(!clickable);
            b.shape.setFillColor(base);
            b.draw(window);

            float cx = b.shape.getPosition().x;
            float cy = b.shape.getPosition().y;

            float top    = cy - b.shape.getSize().y/2.f;
            float bottom = cy + b.shape.getSize().y/2.f;
            float left   = cx - b.shape.getSize().x/2.f + 12;

            // 主人（上排）
            sf::Text host = mkCenterText(font,
                "Host: " + (r.hostName().empty()? "-" : r.hostName()),
                18, sf::Color::Black);
            host.setOrigin(0, host.getOrigin().y);
            host.setPosition(left, top + 18.f);
            window.draw(host);

            // 房間名
            sf::Text rn = mkCenterText(font, r.name, 26, sf::Color::Black);
            rn.setPosition(cx, cy);
            window.draw(rn);

            // 狀態文字
            std::string st;
            if (r.isPlaying())         st = "PLAYING";
            else if (r.isFull())       st = "FULL";
            else if (r.n_players == 0) st = "Empty room";
            else                       st = std::to_string(r.n_players) + " players";

            if (r.isLocked() && !r.isPlaying() && !r.isFull())
                st += " (Locked)";

            sf::Text stTx = mkCenterText(font, st, 18, sf::Color::Black);
            stTx.setOrigin(0, stTx.getOrigin().y);
            stTx.setPosition(left, bottom - 20.f);
            window.draw(stTx);

            // PRIVATE tag
            if (r.isPrivate)
            {
                sf::RectangleShape tag({78,22});
                tag.setFillColor(sf::Color(255,200,200));
                tag.setOutlineColor(sf::Color(180,30,30));
                tag.setOutlineThickness(2);

                float right = cx + b.shape.getSize().x/2.f - 84;
                tag.setPosition(right, top + 8.f);
                window.draw(tag);

                sf::Text t = mkCenterText(font, "PRIVATE", 14,
                                          sf::Color(180,30,30));
                t.setPosition(right + 39, top + 19.f);
                window.draw(t);
            }
        }

        // ---- 右側面板 ----
        window.draw(rightPanel);

        if (selected == -1)
        {
            sf::Text hint = mkCenterText(
                font, "Click a room on the left.", 20,
                sf::Color(100,100,100));
            hint.setPosition(PANEL_X + PANEL_W/2.f,
                             PANEL_Y + PANEL_H/2.f);
            window.draw(hint);
        }
        else
        {
            selectedLabel.draw(window);
            joinBtn.draw(window);

            if (needsKey)
            {
                passLabel.draw(window);

                for (int i = 0; i < PASS_LEN; i++)
                {
                    float x = passBoxX(i);
                    bool focus = (i == (int)keyInput.size() &&
                                  (int)keyInput.size() < PASS_LEN);

                    passBox[i].setPosition(x, PASS_BOX_Y);
                    passBox[i].setOutlineColor(
                        focus ? sf::Color(0,140,255)
                              : sf::Color::Black);
                    passBox[i].setOutlineThickness(focus ? 4 : 3);
                    window.draw(passBox[i]);

                    if (i < (int)keyInput.size())
                    {
                        sf::Text d = mkCenterText(
                            font, std::string(1, keyInput[i]),
                            28, sf::Color::Black);
                        d.setPosition(
                            x + PASS_W/2.f,
                            PASS_BOX_Y + PASS_H/2.f
                        );
                        window.draw(d);
                    }
                }
            }

            if (!errorMsg.empty()) {
                errorLabel.set(errorMsg);
                errorLabel.centerText();
                errorLabel.draw(window);
            }
        }

        window.display();
    }
}
