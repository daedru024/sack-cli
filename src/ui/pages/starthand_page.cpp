#include "ui/pages/starthand_page.hpp"
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

void runStartHandPage(
    sf::RenderWindow& window,
    State&            state,
    EndReason&        reason,
    const std::string& username)
{
    if (state != State::GameStart)
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


    // -------------------------
    // 顯示初始手牌（扣掉 removedCardId）
    // -------------------------
    std::vector<int> myHand;
    for (int i = 0; i <= 9; i++) myHand.push_back(i);

    if (!UI_TEST_MODE) {
        int removed = gameData.removedCardId;
        if (removed != -1) {
            myHand.erase(std::remove(myHand.begin(), myHand.end(), removed), myHand.end());
        }
    }

    HandPanel hand;
    hand.setArea(60.f, 600.f, 430.f);
    hand.setHand(myHand, font);

    

    // -------------------------
    // Title
    // -------------------------
    Label title(&font, "Start Hand Phase", 400, 30, 40,
                sf::Color::Yellow, sf::Color::Black, 2.f);
    title.centerText();

    // -------------------------
    // 廣播訊息：等待他人棄牌
    // -------------------------
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

    updateBroadcast("Waiting for other players to discard...");

    bool hosted = true;

    // -------------------------
    // Main Loop
    // -------------------------
    while (window.isOpen() && state == State::GameStart)
    {
        // -------------------------
        // 接收伺服器訊息
        // -------------------------
        int status = gameData.RecvPlay();

        if ((status == CHOOSE_RABBIT) && hosted) {
            if(gameData.PlayerID() == 0) hosted = false;
            state = State::Discard;
            return;
        }   

        if (status == gameData.PlayNext()) {
            state = State::Game;
            return;
        }

        // -------------------------
        // UI 事件處理
        // -------------------------
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed) window.close();
            if (e.type == sf::Event::Resized) updateBackgroundUI();

            hand.handleClick(e, window);
        }

        // -------------------------
        // Begin Draw
        // -------------------------
        window.setView(uiView);
        window.clear();
        drawBackground(window);

        title.draw(window);
        hand.draw(window);


        if (broadcastPanel.getSize().x > 0) {
            window.draw(broadcastPanel);
        }
        broadcastLabel.draw(window);

        window.display();
    }
}
