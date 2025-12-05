#include "ui/pages/play_phase_page.hpp"
#include "ui/common/ui_common.hpp"
#include "ui/common/ui_background.hpp"
#include "ui/widgets/hand_panel.hpp"
#include "ui/widgets/player_seat.hpp"
#include "ui/layout/table_layout.hpp"
#include "libcliwrap.hpp"
#include "room.hpp"

#include <sstream>
#include <algorithm>

extern sf::View   uiView;
extern GamePlay   gameData;
extern std::vector<Room> rooms;
extern int        currentRoomIndex;
extern bool       UI_TEST_MODE;

using std::string;

/*
===========================================
          PLAY PHASE PAGE
  - 顯示手牌
  - 玩家可點擊出牌
  - 中間會顯示各玩家出的蓋牌 (背面顏色)
  - server 回傳 c {pID} {code} 後，若成功 → 移除手牌
===========================================
*/

void runPlayPhasePage(
    sf::RenderWindow& window,
    State&            state,
    EndReason&        reason,
    const std::string& username)
{
    if (state != State::Game)
        return;

    sf::Font font;
    loadFontSafe(font);

    int roomIdx = currentRoomIndex;
    if (roomIdx < 0 || roomIdx >= (int)rooms.size()) {
        state = State::RoomInfo;
        return;
    }

    Room& room   = gameData.myRoom;
    int nPlayers = room.n_players;
    int myIndex  = gameData.PlayerID();

    // ===============================
    // 座位初始化
    // ===============================
    std::vector<int> seatToPlayer(nPlayers);
    for (int i = 0; i < nPlayers; i++)
        seatToPlayer[i] = (myIndex + i) % nPlayers;

    auto seatPos = computeSeatPositions(nPlayers);
    std::vector<PlayerSeat> seats(nPlayers);

    for (int s = 0; s < nPlayers; ++s) {
        int p = seatToPlayer[s];
        seats[s].init(font, room.playerNames[p], room.colors[p], (p == myIndex), seatPos[s]);
    }

    // ===============================
    // 初始手牌
    // ===============================
    std::vector<int> myHand;
    for (int i = 0; i < 10; i++) {
        if (gameData.HasCard(i)) myHand.push_back(i);
    }

    HandPanel hand;
    hand.setArea(60.f, 600.f, 430.f);
    hand.setHand(myHand, font);

    // ===============================
    // 中央蓋牌區 (每位玩家一格)
    // ===============================
    struct CardSlot {
        bool filled = false;
        int  cardId = -1;   // 用不到卡牌數字，只存位置
        sf::RectangleShape rect;
        sf::Color color;
    };
    std::vector<CardSlot> slots(nPlayers);

    float baseX = 180.f;
    float baseY = 260.f;
    float dx    = 90.f;

    for (int i = 0; i < nPlayers; i++) {
        slots[i].rect.setSize({70,100});
        slots[i].rect.setPosition(baseX + i * dx, baseY);
        int colorIdx = room.colors[ seatToPlayer[i] ];
        slots[i].color = PLAYER_COLORS[colorIdx];
        slots[i].rect.setFillColor(sf::Color(50,50,50)); // 未填時灰色
        slots[i].rect.setOutlineColor(sf::Color::White);
        slots[i].rect.setOutlineThickness(3);
    }

    // ===============================
    // 按鈕：出牌
    // ===============================
    Button playBtn(&font, "PLAY CARD",
                   400, 520, 160, 50, true);
    playBtn.setDisabled(true);

    // ===============================
    // Title
    // ===============================
    Label title(&font, "Play Phase - Round", 400, 40, 26,
                sf::Color::White, sf::Color::Black, 4);
    title.centerText();

    // ===============================
    // 廣播訊息（如：誰出了牌）
    // ===============================
    sf::Text broadcast;
    broadcast.setFont(font);
    broadcast.setCharacterSize(22);
    broadcast.setFillColor(sf::Color::White);
    broadcast.setOutlineColor(sf::Color::Black);
    broadcast.setOutlineThickness(2);
    broadcast.setPosition(160, 150);
    broadcast.setString("");

    // =======================================================
    // 主迴圈
    // =======================================================
    while (window.isOpen() && state == State::Game)
    {
        // ------------------------------
        // 接收伺服器出牌回覆
        // ------------------------------
        int status = gameData.RecvPlay();

        if (status >= 0) {
            // status = 播放這張牌的玩家 ID
            int p = status; // 0-based 玩家
            int seatIndex = (p - myIndex + nPlayers) % nPlayers;

            slots[seatIndex].filled = true;
            slots[seatIndex].rect.setFillColor(slots[seatIndex].color);

            broadcast.setString(room.playerNames[p] + " played a card");
        }
        else if (status == AUTO_PLAYER) {
            broadcast.setString("Auto player played a card");
        }

        // ------------------------------
        // 處理 UI event
        // ------------------------------
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed)
                window.close();
            if (e.type == sf::Event::Resized)
                updateBackgroundUI();

            hand.handleClick(e, window);

            if (playBtn.clicked(e, window)) {
                int idx = hand.selectedIndex();
                if (idx != -1) {
                    int cardId = myHand[idx];

                    if (gameData.Play(cardId)) {
                        // 成功送出 → 先停用按鈕
                        playBtn.setDisabled(true);
                        broadcast.setString("You played card " + std::to_string(cardId));
                    }
                }
            }
        }

        // ------------------------------
        // 更新按鈕狀態
        // ------------------------------
        playBtn.setDisabled(hand.selectedIndex() == -1);

        if (!playBtn.disabled)
            playBtn.update(window);

        // ------------------------------
        // 畫面
        // ------------------------------
        window.setView(uiView);
        window.clear();
        drawBackground(window);

        title.draw(window);

        // 中央蓋牌
        for (auto& s : slots)
            window.draw(s.rect);

        // 你的手牌
        hand.draw(window);

        playBtn.draw(window);

        // 廣播訊息
        window.draw(broadcast);

        window.display();
    }
}
