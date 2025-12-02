#include "ui/pages/game_page.hpp"
#include "ui/common/ui_common.hpp"
#include "ui/common/ui_background.hpp"
#include "ui/widgets/hand_panel.hpp"
#include "ui/widgets/player_seat.hpp"
#include "ui/layout/table_layout.hpp"
#include "libcliwrap.hpp"
#include "room.hpp"

#include <sstream>

extern sf::View   uiView;
extern GamePlay   gameData;
extern std::vector<Room> rooms;
extern int        currentRoomIndex;
extern bool       UI_TEST_MODE;

using std::string;

// 丟棄階段秒數
static const float DISCARD_LIMIT_SEC = 30.f;

void runGamePage(
    sf::RenderWindow& window,
    State&            state,
    EndReason&        reason,
    const std::string& username)
{
    if (state != State::GameStart)
        return;

    sf::Font font;
    loadFontSafe(font);

    // ---- 取得房間 / 玩家資訊 ----
    int roomIdx = currentRoomIndex;
    if (roomIdx < 0 || roomIdx >= (int)rooms.size()) {
        state = State::RoomInfo;
        return;
    }

    Room& room = gameData.myRoom; // server 同步後會是正確房間
    int nPlayers = room.n_players;
    if (nPlayers < 3 || nPlayers > 5) {
        // 不支援的人數，先退回 room info
        state = State::RoomInfo;
        return;
    }

    // 找出自己在 room 裡的 index
    auto& names = room.playerNames;
    int myIndex = -1;
    for (int i = 0; i < nPlayers; ++i) {
        if (names[i] == username) {
            myIndex = i;
            break;
        }
    }
    if (myIndex == -1) {
        state = State::RoomInfo;
        return;
    }

    // ---- 建 seat 順序：seat 0 = 自己，其他依原本順序繞一圈 ----
    std::vector<int> seatToPlayer(nPlayers);
    seatToPlayer[0] = myIndex;
    int seat = 1;
    for (int i = 0; i < nPlayers; ++i) {
        if (i == myIndex) continue;
        seatToPlayer[seat++] = i;
    }

    auto seatPos = computeSeatPositions(nPlayers);

    // ---- PlayerSeat 物件 ----
    std::vector<PlayerSeat> seats(nPlayers);
    for (int s = 0; s < nPlayers; ++s) {
        int p = seatToPlayer[s];
        seats[s].init(
            font,
            names[p],
            room.colors[p],
            (p == myIndex),
            seatPos[s]
        );
    }

    // ---- 自己的手牌（暫時先用 0~9 測試；之後改成 server 發的手牌）----
    std::vector<int> myHand;
    if (UI_TEST_MODE) {
        myHand = {0,1,2,3,4,5,6,7,8,9};
    } else {
        // TODO: 從 gameData / server 取得實際手牌
        myHand = {0,1,2,3,4,5,6,7,8,9};
    }

    HandPanel hand;
    hand.setArea(80.f, 720.f, 440.f);
    hand.setHand(myHand, font);

    // ---- 右下角「得牌堆」框（目前只是空框）----
    sf::RectangleShape wonStackBox;
    wonStackBox.setSize({120.f, 90.f});
    wonStackBox.setPosition(800.f - 140.f, 600.f - 130.f);
    wonStackBox.setFillColor(sf::Color(80,80,80,180));
    wonStackBox.setOutlineColor(sf::Color::White);
    wonStackBox.setOutlineThickness(3.f);

    sf::Text wonLabel = mkCenterText(font, "Won\nStack", 20, sf::Color::White);
    wonLabel.setOutlineColor(sf::Color::Black);
    wonLabel.setOutlineThickness(2.f);
    {
        auto b = wonLabel.getLocalBounds();
        float cx = wonStackBox.getPosition().x + wonStackBox.getSize().x / 2.f;
        float cy = wonStackBox.getPosition().y + wonStackBox.getSize().y / 2.f;
        wonLabel.setOrigin(b.left + b.width / 2.f,
                           b.top  + b.height / 2.f);
        wonLabel.setPosition(cx, cy);
    }

    // ---- 上方標題＆階段說明 ----
    std::string titleStr = room.name +
        "  -  Discard Phase (Choose 1 card to throw away)";

    Label title(&font, titleStr,
                400.f, 40.f, 26,
                sf::Color::White, sf::Color::Black, 4);
    title.centerText();

    // 倒數
    sf::Clock phaseTimer;

    // 確認按鈕
    Button confirmBtn(&font, "CONFIRM",
                      400.f, 520.f, 200.f, 70.f, true);

    // 如果超時要 auto-play：這裡先只作 UI + 結束，之後你可以接上真正的動作
    auto commitDiscard = [&](int cardId) {
        if (cardId == -1) return false;

        // TODO: 在這裡送給 server：
        // 例如改協定成 19 {cardId} or 13(...) 看你們最後決定
        // 現在先只是讓這個 page 結束，回到 RoomInfo 或下一階段
        // 目前：超時 or confirm 之後，先回到 username page / room info
        // 依你 Q1 規劃：超時 → auto-play + reconnect 到 username page
        // 這裡 toy 版：設定 reason 然後回 EndConn

        return true;
    };

    // ============================================================
    // Main loop of this page
    // ============================================================
    while (window.isOpen() && state == State::GameStart)
    {
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed) {
                window.close();
            }

            if (e.type == sf::Event::Resized) {
                updateBackgroundUI();
            }

            // 點手牌
            hand.handleClick(e, window);

            // 按下 Confirm
            if (confirmBtn.clicked(e, window) ||
                (e.type == sf::Event::KeyPressed &&
                 e.key.code == sf::Keyboard::Enter))
            {
                int cid = hand.selectedCardId();
                if (cid != -1) {
                    if (commitDiscard(cid)) {
                        // 目前簡單處理：結束這局，回到 RoomInfo
                        // 之後你可以改成進入出牌階段 State::GamePlay 等
                        state  = State::RoomInfo;
                        reason = EndReason::None;
                        return;
                    }
                }
            }
        }

        // ---- 倒數處理 ----
        float remain = DISCARD_LIMIT_SEC - phaseTimer.getElapsedTime().asSeconds();
        if (remain <= 0.f) {
            // 超時：auto-play + 重連 username page
            // 現在簡單版：直接結束連線
            reason = EndReason::Timeout;
            state  = State::EndConn;
            return;
        }

        // ---- 更新 confirm 按鈕的狀態 ----
        confirmBtn.setDisabled(hand.selectedCardId() == -1);
        if (!confirmBtn.disabled)
            confirmBtn.update(window);

        // ========================================================
        // Draw
        // ========================================================
        window.setView(uiView);
        window.clear();
        drawBackground(window);

        // 標題
        title.draw(window);

        // 玩家名字（座位）
        for (auto& s : seats)
            s.draw(window);

        // 手牌
        hand.draw(window);

        // 得牌堆
        window.draw(wonStackBox);
        window.draw(wonLabel);

        // 倒數文字
        {
            std::ostringstream oss;
            oss << "Time left: " << (int)remain << "s";
            sf::Text t = mkCenterText(font, oss.str(), 22, sf::Color::White);
            t.setOutlineColor(sf::Color::Black);
            t.setOutlineThickness(2.f);
            t.setPosition(400.f, 90.f);
            window.draw(t);
        }

        // 確認按鈕
        confirmBtn.draw(window);

        window.display();
    }
}
