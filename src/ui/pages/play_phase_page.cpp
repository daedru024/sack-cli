#include "ui/pages/play_phase_page.hpp"
#include "ui/common/ui_common.hpp"
#include "ui/common/ui_background.hpp"
#include "ui/widgets/hand_panel.hpp"
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

    float baseX = 360.f;
    float baseY = 280.f;
    float dx    = 60.f;

    for (int i = 0; i < nPlayers; i++) {
        slots[i].rect.setSize({50,70});
        slots[i].rect.setPosition(baseX + i * dx, baseY);
        slots[i].rect.setFillColor(sf::Color(50,50,50)); // 未填時灰色
        slots[i].rect.setOutlineColor(sf::Color::White);
        slots[i].rect.setOutlineThickness(3);
    }

    // ===============================
    // 按鈕：出牌
    // ===============================
    Button playBtn(&font, "PLAY CARD",
                   400, 520, 180, 50, true);
    playBtn.setDisabled(true);

    // ===============================
    // won stack
    // ===============================
    sf::RectangleShape wonStackBox;
    wonStackBox.setSize({120.f, 90.f});
    wonStackBox.setPosition(660.f, 470.f);
    wonStackBox.setFillColor(sf::Color(80, 80, 80, 180));
    wonStackBox.setOutlineColor(sf::Color::White);
    wonStackBox.setOutlineThickness(3);

    sf::Text wonLabel = mkCenterText(font, "Won\nStack", 20, sf::Color::White);
    wonLabel.setOutlineColor(sf::Color::Black);
    wonLabel.setOutlineThickness(2);
    {
        auto b = wonLabel.getLocalBounds();
        float cx = wonStackBox.getPosition().x + wonStackBox.getSize().x / 2;
        float cy = wonStackBox.getPosition().y + wonStackBox.getSize().y / 2;
        wonLabel.setOrigin(b.left + b.width / 2, b.top + b.height / 2);
        wonLabel.setPosition(cx, cy);
    }

    // ===============================
    // Title
    // ===============================
    Label title(&font, "Play Phase - Round", 400, 40, 28,
                sf::Color::White, sf::Color::Black, 4);
    title.centerText();

    // ===============================
    // 廣播訊息（如：誰出了牌）
    // ===============================
    sf::Text broadcast;
    broadcast.setFont(font);
    broadcast.setCharacterSize(32);
    broadcast.setFillColor(sf::Color::Black);
    broadcast.setOutlineColor(sf::Color::White);
    broadcast.setOutlineThickness(4);
    broadcast.setPosition(260, 70);
    broadcast.setString("");

    // ===============================
    // 新增 UI: 金錢與倒數計時
    // ===============================
    sf::Clock stepTimer; // 倒數計時器

    // 金錢顯示 (左下角)
    sf::Text moneyText = mkCenterText(font, "$", 28, sf::Color::Yellow);
    moneyText.setOutlineColor(sf::Color::Black);
    moneyText.setOutlineThickness(2);
    moneyText.setPosition(100.f, 550.f); 

    // 倒數計時顯示 (下方中間，手牌下方)
    sf::Text timerText = mkCenterText(font, "60s", 26, sf::Color::White);
    timerText.setOutlineColor(sf::Color::Black);
    timerText.setOutlineThickness(2);
    timerText.setPosition(400.f, 580.f);

    // 紀錄場上已出牌數量
    int playedCount = 0;

    // 追蹤當前輪到誰出牌 (預設為 0 / Host)
    int currentTurnPlayer = 0;

    // 如果一開始就是我的回合，重置計時器
    if (currentTurnPlayer == myIndex) {
        stepTimer.restart();
    }

    // =======================================================
    // 主迴圈
    // =======================================================
    while (window.isOpen() && state == State::Game)
    {
        int status = gameData.RecvPlay();
        // add status < 6 to confirm pID
        if (status >= 0 && status < 6) {
            // status = 播放這張牌的玩家 ID
            int p = status; // 0-based 玩家
            
            // 依序填入最左側的空位
            if (playedCount < nPlayers) {
                // 取得該玩家顏色
                int cIdx = room.colors[p];
                sf::Color pColor =  PLAYER_COLORS[cIdx];

                slots[playedCount].filled = true;
                slots[playedCount].rect.setFillColor(pColor);
                playedCount++;
            }

            broadcast.setString(room.playerNames[p] + " played a card");

            // 更新回合：換下一個人
            currentTurnPlayer = (p + 1) % nPlayers;

            // 如果換我了，重置計時器
            if (currentTurnPlayer == myIndex) {
                stepTimer.restart();
                playBtn.setDisabled(false); // 確保輪到自己時按鈕狀態正確(雖然下面有更新邏輯)
            }

        }
        else if (status == AUTO_PLAYER) {
            broadcast.setString("Auto player played a card");
            // maybe "someone left the room"? 
        }

        // -------------------
        // 更新 UI 數據
        // -------------------
        
        bool isMyTurn = (currentTurnPlayer == myIndex);

        // 更新倒數時間 (只在輪到自己時顯示)
        if (isMyTurn) {
            int remain = 45 - (int)stepTimer.getElapsedTime().asSeconds();
            if (remain < 0) remain = 0;
            
            timerText.setString("Time left - " + std::to_string(remain) + "s");
            
            // 重新置中
            sf::FloatRect b = timerText.getLocalBounds();
            timerText.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
            timerText.setPosition(400.f, 580.f);
            
            // 倒數少於 10 秒變紅色提醒
            if (remain <= 10) timerText.setFillColor(sf::Color::Red);
            else timerText.setFillColor(sf::Color::White);
        } else {
            timerText.setString(""); // 非回合不顯示
        }

        // 更新金錢
        moneyText.setString("Money: $" + std::to_string(gameData.Money()));
        {
            // 靠左對齊
            sf::FloatRect b = moneyText.getLocalBounds();
            moneyText.setOrigin(b.left, b.top + b.height/2.f); 
            moneyText.setPosition(60.f, 550.f); // 手牌區左側下方
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

            //if (playBtn.clicked(e, window)) {
            if (playBtn.clicked(e, window) && isMyTurn) {
                int cardId = hand.selectedCardId();
                if (cardId != -1) {
                    if (gameData.Play(cardId)) {
                        playBtn.setDisabled(true);
                        broadcast.setString("You played card " + std::to_string(cardId));
                        auto it = std::find(myHand.begin(), myHand.end(), cardId);
                        if (it != myHand.end()) {
                            myHand.erase(it);
                        }
                        hand.setHand(myHand, font);
                        hand.clearSelection();
                    }
                }
            }
        }

        // ------------------------------
        // 更新按鈕狀態
        // ------------------------------
        bool canPlay = isMyTurn && (hand.selectedIndex() != -1);
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

        // 只在輪到自己時畫倒數計時
        if (isMyTurn) {
            window.draw(timerText);
        }
        
        window.draw(moneyText);
        window.draw(wonStackBox);
        window.draw(wonLabel);

        window.display();
    }
}
