#include "ui/pages/game_page.hpp"
#include "ui/common/ui_common.hpp"
#include "ui/common/ui_background.hpp"
#include "ui/widgets/hand_panel.hpp"
#include "ui/widgets/player_seat.hpp"
#include "ui/layout/table_layout.hpp"
#include "libcliwrap.hpp"
#include "room.hpp"

#include <sstream>

extern sf::View uiView;
extern GamePlay gameData;
extern std::vector<Room> rooms;
extern int currentRoomIndex;
extern bool UI_TEST_MODE;

using std::string;

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

    int roomIdx = currentRoomIndex;
    if (roomIdx < 0 || roomIdx >= (int)rooms.size()) {
        state = State::RoomInfo;
        return;
    }

    Room& room = gameData.myRoom;
    int nPlayers = room.n_players;
    if (nPlayers < 3 || nPlayers > 5) {
        state = State::RoomInfo;
        return;
    }

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

    std::vector<int> seatToPlayer(nPlayers);
    seatToPlayer[0] = myIndex;
    int seat = 1;
    for (int i = 0; i < nPlayers; i++) {
        if (i == myIndex) continue;
        seatToPlayer[seat++] = i;
    }

    auto seatPos = computeSeatPositions(nPlayers);

    std::vector<PlayerSeat> seats(nPlayers);
    for (int s = 0; s < nPlayers; ++s) {
        int p = seatToPlayer[s];
        seats[s].init(font, names[p], room.colors[p], (p == myIndex), seatPos[s]);
    }

    std::vector<int> myHand;
    if (UI_TEST_MODE)
        myHand = {0,1,2,3,4,5,6,7,8,9};
    else
        myHand = {0,1,2,3,4,5,6,7,8,9};  // TODO: 用 server 的手牌

    HandPanel hand;
    hand.setArea(60.f, 600.f, 430.f);
    hand.setHand(myHand, font);

    sf::RectangleShape wonStackBox;
    wonStackBox.setSize({120.f, 90.f});
    wonStackBox.setPosition(660.f, 470.f);
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
        wonLabel.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
        wonLabel.setPosition(cx, cy);
    }

    string titleStr = room.name + "  -  Discard Phase";
    Label title(&font, titleStr, 400.f, 40.f, 26,
                sf::Color::White, sf::Color::Black, 4);
    title.centerText();

    sf::Clock phaseTimer;

    Button confirmBtn(&font, "CONFIRM",
                      400.f, 520.f, 160.f, 50.f, true);

    bool hasCommitted = false;

    auto commitDiscard = [&](int cardId) {
        if (cardId == -1) return false;
        std::stringstream ss;
        ss << "19 " << cardId;
        Write(gameData.Sockfd(), ss.str().c_str(), ss.str().length());
        return true;
    };

    while (window.isOpen() && state == State::GameStart)
    {
        int status = gameData.RecvPlay();

        if (status != -2) {
            state = State::Game;
            reason = EndReason::None;
            return;
        }

        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed)
                window.close();

            if (e.type == sf::Event::Resized)
                updateBackgroundUI();

            hand.handleClick(e, window);

            if(!hasCommitted){
                if (confirmBtn.clicked(e, window) ||
                    (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Enter))
                {
                    int cid = hand.selectedCardId();
                    if (cid != -1) {
                        if (commitDiscard(cid)) {
                            hasCommitted = true;
                            //confirmBtn.setDisabled(true);
                            hand.clearSelection();
                        }
                    }
                }
            }
            
        }

        float remain = DISCARD_LIMIT_SEC - phaseTimer.getElapsedTime().asSeconds();
        if (remain <= 0.f) {
            reason = EndReason::Timeout;
            state  = State::EndConn;
            return;
        }

        

        if (hasCommitted){
            confirmBtn.setDisabled(true);
        }
        else confirmBtn.setDisabled(hand.selectedCardId() == -1);

        if (!confirmBtn.disabled)
            confirmBtn.update(window);

        window.setView(uiView);
        window.clear();
        drawBackground(window);

        title.draw(window);

        for (auto& s : seats)
            s.draw(window);

        hand.draw(window);

        window.draw(wonStackBox);
        window.draw(wonLabel);

        {
            std::ostringstream oss;
            oss << "Time left: " << (int)remain << "s";
            sf::Text t = mkCenterText(font, oss.str(), 22, sf::Color::White);
            t.setOutlineColor(sf::Color::Black);
            t.setOutlineThickness(2.f);
            t.setPosition(400.f, 90.f);
            window.draw(t);
        }

        confirmBtn.draw(window);

        window.display();
    }
}
