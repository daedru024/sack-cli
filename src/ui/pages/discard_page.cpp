#include "ui/pages/discard_page.hpp"
#include "ui/common/ui_common.hpp"
#include "ui/common/ui_background.hpp"
#include "ui/widgets/hand_panel.hpp"
#include "ui/widgets/label.hpp"
#include "ui/widgets/button.hpp"
#include "libcliwrap.hpp"
#include "room.hpp"


#include <sstream>
#include <iostream>

extern sf::View   uiView;
extern GamePlay   gameData;
extern std::vector<Room> rooms;
extern int        currentRoomIndex;
extern bool       UI_TEST_MODE;

using std::string;

static const float DISCARD_LIMIT_SEC = 30.f;

void runDiscardPage(
    sf::RenderWindow& window,
    State&            state,
    EndReason&        reason,
    const std::string& username)
{
    
    if (state != State::Discard){
        return;
    }
    sf::Font font;
    loadFontSafe(font);

    int roomIdx = currentRoomIndex;
    if (roomIdx < 0 || roomIdx >= (int)rooms.size()) {
        state = State::RoomInfo;
        return;
    }

    Room& room = gameData.myRoom;
    int nPlayers = room.n_players;
    int myIndex = gameData.PlayerID();

    if (myIndex == -1) {
        state = State::RoomInfo;
        return;
    }

    // 2. 準備目標手牌 (盲選模式)
    // 目標是下一位玩家
    int targetPId = (myIndex + 1) % nPlayers;
    int targetColorIdx = room.colors[targetPId];

    HandPanel targetHand;
    // 使用與 game_page 相同的 UI 位置
    targetHand.setArea(60.f, 600.f, 430.f); 
    targetHand.setBlindMode(true); 

    // 生成假手牌 (10張)
    std::vector<int> dummyHand(10, -1);  // 0 -> -1
    targetHand.setHand(dummyHand, font);

    // 設定牌背顏色 (目標玩家顏色)
    sf::Color backColor = sf::Color(100, 100, 100);
    if (targetColorIdx >= 0 && targetColorIdx < (int)PLAYER_COLORS.size()) {
        backColor = PLAYER_COLORS[targetColorIdx];
    }

    for (auto& c : targetHand.cards) {
        c.isTextureMode = false;
        c.rect.setFillColor(backColor);
        // c.text.setString(""); 
    }

    // 3. UI 元件
    Label title(&font, "Discard Phase", 400.f, 30.f, 40, sf::Color::Yellow, sf::Color::Black, 2.f);
    title.centerText();

    Label broadcastLabel(&font, "", 400, 110, 32, sf::Color::Black);
    broadcastLabel.text.setOutlineColor(sf::Color::White);
    broadcastLabel.text.setOutlineThickness(4);

    sf::RectangleShape broadcastPanel;
    broadcastPanel.setFillColor(sf::Color(50, 45, 40, 180));
    broadcastPanel.setOutlineColor(sf::Color(255, 255, 255, 120)); 
    broadcastPanel.setOutlineThickness(1.f);

    auto updateBroadcast = [&](std::string msg) {
        if (msg.empty()) {
            broadcastLabel.set("");
            broadcastPanel.setSize({0, 0});
            return;
        }

        broadcastLabel.set(msg);
        
        sf::FloatRect textBounds = broadcastLabel.text.getLocalBounds();
        
        float paddingX = 40.f;
        float paddingY = 20.f;
        float width = textBounds.width + paddingX * 2.f;
        float height = textBounds.height + paddingY * 2.f;
        
        broadcastPanel.setSize({width, height});
        broadcastPanel.setOrigin(width / 2.f, height / 2.f);
        broadcastPanel.setPosition(400.f, 115.f); 
        
        broadcastLabel.text.setPosition(400.f, 115.f);
        broadcastLabel.centerText();
    };

    updateBroadcast("Pick Next Player's Card");

    sf::Clock phaseTimer;

    Button confirmBtn(&font, "CONFIRM",
                      400.f, 530.f, 160.f, 50.f, true); 

    bool hasCommitted = false;
    bool gotDiscarded = false;

    // 提交函式 (傳入 Index)
    auto commitDiscard = [&](int index) {
        if (index == -1) return false;
        gameData.Rabbit(index);
        return true;
    };

    while (window.isOpen() && state == State::Discard)
    {
        int status = gameData.RecvPlay();
        //if (status == CHOOSE_RABBIT) { 
        if (gameData.removedCardId != -1) {
            gotDiscarded = true; 
        }

        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed) window.close();
            if (e.type == sf::Event::Resized) updateBackgroundUI();

            targetHand.handleClick(e, window);

            if (!hasCommitted) 
            {
                if (confirmBtn.clicked(e, window) ||
                    (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Enter))
                {
                    int idx = targetHand.selectedIndex(); 
                    if (idx != -1) {
                        if (commitDiscard(idx)) {
                            hasCommitted = true;
                            confirmBtn.setDisabled(true);
                            targetHand.clearSelection();
                        }
                    }
                }
            }
        }

        // delete gotDiscarded
        if (hasCommitted) {
            state = State::GameStart;
            return;
        }

        float remain = DISCARD_LIMIT_SEC - phaseTimer.getElapsedTime().asSeconds();
        if (remain <= 0.f) {
            reason = EndReason::Timeout;
            state  = State::EndConn;
            return;


        }

        // 2. 更新按鈕狀態
        if (hasCommitted) {
            confirmBtn.setDisabled(true);
        } else {
            // 未提交時：沒選牌就禁用，有選牌就啟用
            confirmBtn.setDisabled(targetHand.selectedIndex() == -1);
        }
        
        // 只有未被禁用時才 update (處理 hover)
        if (!confirmBtn.disabled) 
            confirmBtn.update(window);

        


        window.setView(uiView);
        window.clear();
        drawBackground(window);

        title.draw(window);
        
        if (broadcastPanel.getSize().x > 0) {
            window.draw(broadcastPanel);
        }
        broadcastLabel.draw(window);

        targetHand.draw(window);

        {
            std::ostringstream oss;
            oss << "Time left: " << (int)remain << "s";
            sf::Text t = mkCenterText(font, oss.str(), 26, sf::Color::White);
            t.setOutlineColor(sf::Color::Black);
            t.setOutlineThickness(2.f);
            t.setPosition(400.f, 580.f);
            t.setFillColor(remain <= 10 ? sf::Color::Red : sf::Color::White);
            window.draw(t);
        }

        confirmBtn.draw(window);
        window.display();
    }
}