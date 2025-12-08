#include "ui/pages/play_phase_page.hpp"
#include "ui/common/ui_common.hpp"
#include "ui/common/ui_background.hpp"
#include "ui/widgets/hand_panel.hpp"
#include "ui/widgets/bid_panel.hpp" 
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


void runPlayPhasePage(
    sf::RenderWindow& window,
    State&            state,
    EndReason&        reason,
    const std::string& username)
{
    if (state != State::Game) return;

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

    // 手牌初始化
    std::vector<int> myHand;
    for (int i = 0; i < 10; i++) {
        if (gameData.HasCard(i)) myHand.push_back(i);
    }

    HandPanel hand;
    hand.setArea(60.f, 600.f, 430.f);
    hand.setHand(myHand, font);

    // 中央蓋牌區 (位置調整：往上移)
    struct CardSlot {
        bool filled = false;
        bool revealed = false; // Add revealed state
        int cardValue = 0;     // Store value for display
        sf::RectangleShape rect;
        sf::Color color;
        sf::Text text;         // Text for revealed value
    };
    std::vector<CardSlot> slots(nPlayers);
    
    float baseX = 260.f;
    float baseY = 180.f; 
    float dx    = 60.f;

    for (int i = 0; i < nPlayers; i++) {
        slots[i].rect.setSize({50,70});
        slots[i].rect.setPosition(baseX + i * dx, baseY);
        slots[i].rect.setFillColor(sf::Color(50,50,50));
        slots[i].rect.setOutlineColor(sf::Color::White);
        slots[i].rect.setOutlineThickness(3);

        slots[i].text.setFont(font);             
        slots[i].text.setCharacterSize(30);      
        slots[i].text.setFillColor(sf::Color::Black); 
    }

    // Play Card 按鈕
    Button playBtn(&font, "PLAY CARD", 400, 520, 180, 50, true);
    playBtn.setDisabled(true);

    // ★★★ 初始化 BidPanel ★★★
    BidPanel bidPanel;
    bidPanel.init(font, 130.f, 320.f); 
    bidPanel.setVisible(false);

    // Won Stack & Labels
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

    Label title(&font, "Play Phase - Round", 400, 40, 28, sf::Color::White, sf::Color::Black, 4);
    title.centerText();

    // 廣播訊息
    sf::Text broadcast;
    broadcast.setFont(font);
    broadcast.setCharacterSize(32);
    broadcast.setFillColor(sf::Color::Black);
    broadcast.setOutlineColor(sf::Color::White);
    broadcast.setOutlineThickness(4);
    broadcast.setPosition(275, 70); 
    broadcast.setString("");

    sf::Clock stepTimer;
    sf::Text moneyText = mkCenterText(font, "$", 28, sf::Color::Yellow);
    moneyText.setOutlineColor(sf::Color::Black);
    moneyText.setOutlineThickness(2);
    moneyText.setPosition(100.f, 550.f);

    sf::Text timerText = mkCenterText(font, "60s", 26, sf::Color::White);
    timerText.setOutlineColor(sf::Color::Black);
    timerText.setOutlineThickness(2);
    timerText.setPosition(400.f, 580.f);

    // 狀態變數
    int playedCount = 0;
    int currentTurnPlayer = 0;
    if (currentTurnPlayer == myIndex) stepTimer.restart();

    bool isBiddingPhase = false;
    int currentHighBid = 0;

    // 標記本回合是否已送出bid
    bool hasSubmitted = false;

    // Track how many cards have been revealed by passing
    int revealedCount = 0;

    // Main Loop
    while (window.isOpen() && state == State::Game)
    {
        // ---------------------------------------------------
        //  1. 網路封包接收
        // ---------------------------------------------------
        if (!isBiddingPhase) 
        {
            // --- 出牌階段 ---
            int status = gameData.RecvPlay();
            if (status >= 0 && status < 6) {
                int p = status;
                if (playedCount < nPlayers) {
                    int cIdx = room.colors[p];
                    sf::Color pColor = PLAYER_COLORS[cIdx];
                    slots[playedCount].filled = true;
                    slots[playedCount].rect.setFillColor(pColor);
                    playedCount++;
                }
                broadcast.setString(room.playerNames[p] + " played a card");
                stepTimer.restart();
                currentTurnPlayer = (p + 1) % nPlayers;

                if (playedCount == nPlayers) {
                    isBiddingPhase = true;
                    broadcast.setString("Bidding Phase Start");
                    playBtn.setDisabled(true); 
                    currentHighBid = 0; 
                    hasSubmitted = false;
                    revealedCount = 0;
                }
            }
            else if (status == AUTO_PLAYER) {
                broadcast.setString("Auto player played");
            }
        }
        else 
        {
            // --- 競標階段 ---
            auto result = gameData.RecvBid();
            int nextPlayer = result.first;
            int bidder     = result.second.first;
            int amount     = result.second.second;
            int revealedId = gameData.CardsPlayed[gameData.PlayedThisRound()-1];

            if (nextPlayer != -2 && nextPlayer != CONN_CLOSED && nextPlayer != AUTO_PLAYER) {
                stepTimer.restart();

                // 如果換人了，代表我的操作(如果有的話)已經被 Server 確認，可以重置提交狀態
                if (currentTurnPlayer != nextPlayer) {
                    hasSubmitted = false;
                }

                currentTurnPlayer = nextPlayer;

                if (bidder >= 0) {
                    if (amount > 0) {
                        currentHighBid = amount;
                        broadcast.setString(room.playerNames[bidder] + " bid $" + std::to_string(amount));
                    } else {
                        broadcast.setString(room.playerNames[bidder] + " passed");

                        // Reveal the leftmost unrevealed card (slots[revealedCount])
                        if (revealedId != -1 && revealedCount < nPlayers) {
                            CardSlot& slot = slots[revealedCount];
                            slot.revealed = true;
                            
                            // Set color (based on card type)
                            CardType t = getCardType(revealedId);
                            slot.rect.setFillColor(cardFillColor(t)); 
                            
                            // Set text value
                            int val = cardValue(revealedId);
                            slot.text.setString(std::to_string(val));
                            
                            // Center text
                            sf::FloatRect b = slot.text.getLocalBounds();
                            slot.text.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
                            sf::Vector2f pos = slot.rect.getPosition();
                            sf::Vector2f sz = slot.rect.getSize();
                            slot.text.setPosition(pos.x + sz.x/2.f, pos.y + sz.y/2.f);
                            
                            revealedCount++;
                        }
                    }
                }

                if (room.inGame % 2 != 0) {
                    isBiddingPhase = false;
                    playedCount = 0;
                    currentHighBid = 0;
                    revealedCount = 0;
                    for (auto& s : slots) {
                        s.filled = false;
                        s.revealed = false;
                        s.rect.setFillColor(sf::Color(50, 50, 50));
                    }
                    broadcast.setString("New Round Start");
                    bidPanel.setVisible(false);
                    hasSubmitted = false;
                }
            }
        }

        // ---------------------------------------------------
        //  2. UI 邏輯
        // ---------------------------------------------------
        bool isMyTurn = (currentTurnPlayer == myIndex);

        if (isBiddingPhase) {
            if (isMyTurn && !hasSubmitted) {
                bidPanel.setVisible(true);
                bidPanel.setRange(currentHighBid + 1, gameData.Money());
                broadcast.setString("Your turn to Bid");
            } else {
                bidPanel.setVisible(false);
            }
        } else {
            bidPanel.setVisible(false);
        }

        if (isMyTurn) {
            int remain = (isBiddingPhase ? 60 : 45) - (int)stepTimer.getElapsedTime().asSeconds();
            if (remain < 0) remain = 0;
            timerText.setString("Time left - " + std::to_string(remain) + " s");
            // 置中邏輯
            sf::FloatRect b = timerText.getLocalBounds();
            timerText.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
            timerText.setPosition(400.f, 580.f);
            timerText.setFillColor(remain <= 10 ? sf::Color::Red : sf::Color::White);
        } else {
            timerText.setString("");
        }

        moneyText.setString("Money: $" + std::to_string(gameData.Money()));
        {
            // 靠左對齊
            sf::FloatRect b = moneyText.getLocalBounds();
            moneyText.setOrigin(b.left, b.top + b.height/2.f); 
            moneyText.setPosition(60.f, 550.f); // 手牌區左側下方
        }

        // ---------------------------------------------------
        //  3. 事件
        // ---------------------------------------------------
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed) window.close();
            if (e.type == sf::Event::Resized) updateBackgroundUI();

            if (!isBiddingPhase) 
            {
                hand.handleClick(e, window);
                if (playBtn.clicked(e, window) && isMyTurn) {
                    int cardId = hand.selectedCardId();
                    if (cardId != -1) {
                        if (gameData.Play(cardId)) {
                            playBtn.setDisabled(true);
                            broadcast.setString("You played " + std::to_string(cardId));
                            auto it = std::find(myHand.begin(), myHand.end(), cardId);
                            if (it != myHand.end()) myHand.erase(it);
                            hand.setHand(myHand, font);
                            hand.clearSelection();
                        }
                    }
                }
            } 
            else 
            {
                // 處理競標面板事件
                auto action = bidPanel.handleEvent(e, window);
                if (action == BidPanel::Action::Submit) {
                    gameData.SendBid(bidPanel.value);
                    bidPanel.setVisible(false);
                    hasSubmitted = true;
                }
                else if (action == BidPanel::Action::Pass) {
                    gameData.SendBid(0); // 棄標
                    bidPanel.setVisible(false);
                    hasSubmitted = true;
                }
            }
        }

        // Play Button 狀態
        if (!isBiddingPhase) {
            bool canPlay = isMyTurn && (hand.selectedIndex() != -1);
            playBtn.setDisabled(!canPlay);
            if (!playBtn.disabled) playBtn.update(window);
        }

        // ---------------------------------------------------
        //  4. 繪圖
        // ---------------------------------------------------
        window.setView(uiView);
        window.clear();
        drawBackground(window);

        title.draw(window);
        
        for (auto& s : slots){
            window.draw(s.rect);
            if (s.revealed) {
                window.draw(s.text);
            }
        }
        
        // 出牌階段亮，競標階段暗 (可透過 Color 調整，這裡簡單處理)
        hand.draw(window); 

        if (!isBiddingPhase) playBtn.draw(window);
        if (isBiddingPhase) bidPanel.draw(window);

        window.draw(broadcast);
        if (isMyTurn) window.draw(timerText);
        window.draw(moneyText);
        window.draw(wonStackBox);
        window.draw(wonLabel);

        window.display();
    }
}