#include "ui/pages/starthand_page.hpp"
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

    // if (gameData.startFlag == CHOOSE_RABBIT) {
    //     state = State::Discard;
    //     return;
    // }

    int roomIdx = currentRoomIndex;
    if (roomIdx < 0 || roomIdx >= (int)rooms.size()) {
        state = State::RoomInfo;
        return;
    }

    Room& room   = gameData.myRoom;
    int nPlayers = room.n_players;
    int myIndex  = gameData.PlayerID();

    // // -------------------------
    // // 建立座位資訊（不會變）
    // // -------------------------
    // std::vector<int> seatToPlayer(nPlayers);
    // for (int i = 0; i < nPlayers; i++)
    //     seatToPlayer[i] = (myIndex + i) % nPlayers;

    // auto seatPos = computeSeatPositions(nPlayers);
    // std::vector<PlayerSeat> seats(nPlayers);

    // for (int s = 0; s < nPlayers; ++s) {
    //     int p = seatToPlayer[s];
    //     seats[s].init(font, room.playerNames[p], room.colors[p], (p == myIndex), seatPos[s]);
    // }

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
    // UI: Won Stack (未開始使用)
    // -------------------------
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

    // -------------------------
    // Title
    // -------------------------
    Label title(&font, "Start Hand Phase", 400, 40, 26,
                sf::Color::White, sf::Color::Black, 4);
    title.centerText();

    // -------------------------
    // 廣播訊息：等待他人棄牌
    // -------------------------
    sf::Text waitMsg;
    waitMsg.setFont(font);
    waitMsg.setString("Waiting for other players to discard...");
    waitMsg.setCharacterSize(22);
    waitMsg.setFillColor(sf::Color::White);
    waitMsg.setOutlineColor(sf::Color::Black);
    waitMsg.setOutlineThickness(2);
    waitMsg.setPosition(400 - waitMsg.getLocalBounds().width / 2, 90);

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

        // ⭕ Case 1：要去棄牌
        //if ((status == CHOOSE_RABBIT || gameData.PlayerID() == 0) && hosted) {
        if ((status == CHOOSE_RABBIT) && hosted) {
            if(gameData.PlayerID() == 0) hosted = false;
            state = State::Discard;
            return;
        }   

        // ⭕ Case 2：當 Host 最後收到 ri → 出牌階段開始
        if (status == gameData.PlayNext()) {
            state = State::Game; // runPlayPhasePage() 在 main 裡處理
            return;
        }

        // 其餘 status = -1 / AUTO_PLAYER → 忽略即可

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

        window.draw(wonStackBox);
        window.draw(wonLabel);

        window.draw(waitMsg);

        window.display();
    }
}
