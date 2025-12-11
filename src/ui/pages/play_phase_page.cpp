#include "ui/pages/play_phase_page.hpp"
#include "ui/common/ui_common.hpp"
#include "ui/common/ui_background.hpp"
#include "ui/widgets/hand_panel.hpp"
#include "ui/widgets/bid_panel.hpp" 
#include "libcliwrap.hpp"
#include "room.hpp"

#include "ui/widgets/card_widget.hpp"
#include "ui/widgets/label.hpp"
#include "ui/widgets/game_cards.hpp"

#include <sstream>
#include <algorithm>

extern sf::View   uiView;
extern GamePlay   gameData;
extern std::vector<Room> rooms;
extern int        currentRoomIndex;

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

    // --- 手牌初始化 ---
    std::vector<int> myHand;
    for (int i = 0; i < 10; i++) {
        if (gameData.HasCard(i)) myHand.push_back(i);
    }

    HandPanel hand;
    hand.setArea(60.f, 600.f, 430.f);
    hand.setHand(myHand, font);

    // --- 中央卡牌區 (使用 CardWidget) ---
    struct TableSlot {
        CardWidget widget;
        bool       filled   = false;
        bool       revealed = false; 
        Label      moneyLabel;


        TableSlot(const sf::Font* f) : moneyLabel(f, "", 0, 0, 20, sf::Color::Yellow) {
            moneyLabel.text.setOutlineColor(sf::Color::Black);
            moneyLabel.text.setOutlineThickness(1.5f);
        }
    };

    std::vector<TableSlot> slots;
    slots.reserve(nPlayers);
    for(int i=0; i<nPlayers; ++i) {
        slots.emplace_back(&font);
    }
    
    float baseX = 280.f;
    float baseY = 200.f; 
    float dx    = 60.f;

    std::vector<int> makeupMoney;
    if (nPlayers == 3 || nPlayers == 4) makeupMoney = {2, 4, 6, 0};
    else if (nPlayers == 5) makeupMoney = {2, 3, 4, 6, 0};

    for (int i = 0; i < nPlayers; i++) {
        sf::Vector2f pos(baseX + i * dx, baseY);
        sf::Vector2f size(50.f, 70.f);
        
        slots[i].widget.init(font, 2, pos, size); // 2 是背面/預設
        slots[i].widget.rect.setFillColor(sf::Color(50, 50, 50)); // 未填充時是灰色
        slots[i].widget.text.setString(""); // 不顯示文字

        int amount = makeupMoney[i];
        if (amount > 0) {
            slots[i].moneyLabel.set("+" + std::to_string(amount));
        } else {
            slots[i].moneyLabel.set("");
        }

        sf::FloatRect b = slots[i].moneyLabel.text.getLocalBounds();
        slots[i].moneyLabel.text.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
        slots[i].moneyLabel.text.setPosition(pos.x + size.x/2.f, pos.y - 20.f);
    }

    // Play Card 按鈕
    Button playBtn(&font, "PLAY CARD", 400, 520, 180, 50, true);
    playBtn.setDisabled(true);

    // BidPanel
    BidPanel bidPanel;
    bidPanel.init(font, 130.f, 330.f); 
    bidPanel.setVisible(false);

    // --- (Won Stack) ---
    sf::RectangleShape wonStackBg;
    wonStackBg.setSize({120.f, 90.f});
    wonStackBg.setPosition(660.f, 470.f);
    wonStackBg.setFillColor(sf::Color(80, 80, 80, 180));
    wonStackBg.setOutlineThickness(0); 

    sf::RectangleShape wonStackFrame = wonStackBg;
    wonStackFrame.setFillColor(sf::Color::Transparent);
    wonStackFrame.setOutlineColor(sf::Color::White);
    wonStackFrame.setOutlineThickness(3);

    sf::RectangleShape wonCardRect;
    wonCardRect.setSize({70.f, 100.f}); 
    wonCardRect.setFillColor(sf::Color(255, 245, 180)); // 普通貓顏色
    wonCardRect.setOutlineColor(sf::Color::Black);
    wonCardRect.setOutlineThickness(2);

    Label countLabel(&font, "", 0, 0, 24, sf::Color::Cyan); // 位置稍後設定
    countLabel.text.setOutlineColor(sf::Color::Black);
    countLabel.text.setOutlineThickness(2);

    Label wonLabel(&font, "Won\nStack", 0, 0, 20, sf::Color::White);
    wonLabel.text.setOutlineColor(sf::Color::Black);
    wonLabel.text.setOutlineThickness(2);
    {
        float cx = wonStackBg.getPosition().x + wonStackBg.getSize().x / 2;
        float cy = wonStackBg.getPosition().y + wonStackBg.getSize().y / 2;
        wonLabel.text.setPosition(cx, cy);
        wonLabel.centerText();
    }

    // --- 4. 文字標籤 (使用 Label) ---
    std::string roundStr = "Play Phase - Round " + std::to_string(gameData.Round());
    Label titleLabel(&font, roundStr, 400, 40, 28, sf::Color::Yellow, sf::Color::Black, 2.f);
    titleLabel.centerText();

    // 廣播訊息
    Label broadcastLabel(&font, "", 400, 100, 32, sf::Color::Black);
    broadcastLabel.text.setOutlineColor(sf::Color::White);
    broadcastLabel.text.setOutlineThickness(4);

    // 封裝廣播更新函式
    auto updateBroadcast = [&](std::string msg) {
        broadcastLabel.set(msg);
        broadcastLabel.centerText();
    };
    updateBroadcast("");

    std::string lastActionMsg = "";

    sf::Clock stepTimer;
    sf::Clock resultTimer;
    const float SHOW_DURATION = 10.0f;

    Label moneyLabel(&font, "$", 100, 550, 28, sf::Color::Yellow);
    moneyLabel.text.setOutlineColor(sf::Color::Black);
    moneyLabel.text.setOutlineThickness(2);

    Label timerLabel(&font, "60s", 400, 580, 26, sf::Color::White);
    timerLabel.text.setOutlineColor(sf::Color::Black);
    timerLabel.text.setOutlineThickness(2);

    // 狀態變數
    int playedCount = 0;
    int currentTurnPlayer = 0;
    if (currentTurnPlayer == myIndex) stepTimer.restart();

    bool isBiddingPhase = false;
    bool showingResult = false;

    int currentHighBid = 0;
    int currentHighBidder = -1;
    bool hasSubmitted = false;
    int revealedCount = 0;
    int wonRoundsCount = 0;

    // Main Loop
    while (window.isOpen() && state == State::Game)
    {
        // ---------------------------------------------------
        //  1. 網路封包接收
        // ---------------------------------------------------
        if (!showingResult) 
        {
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
                        slots[playedCount].widget.rect.setFillColor(pColor);
                        playedCount++;
                    }
                    lastActionMsg = room.playerNames[p] + " played a card";
                    stepTimer.restart();
                    currentTurnPlayer = (p + 1) % nPlayers;

                    if (playedCount == nPlayers) {
                        isBiddingPhase = true;
                        lastActionMsg = "Bidding Phase Start";
                        playBtn.setDisabled(true); 
                        currentHighBid = 0; 
                        hasSubmitted = false;
                        revealedCount = 0;
                    }
                }
                else if(status == CONN_CLOSED) {
                    //handle closed connection
                    state = State::EndConn;
                    return;
                }
            }
            else 
            {
                // --- 競標階段 ---
                while(true){
                    auto result = gameData.RecvBid();
                    int nextPlayer = result.first;
                    int bidder     = result.second.first;
                    int amount     = result.second.second;
                    
                    if (nextPlayer == -2) break; 
                    if (nextPlayer == CONN_CLOSED) {
                        state = State::EndConn;
                        return;
                    }

                    stepTimer.restart();
                    if (currentTurnPlayer != nextPlayer) hasSubmitted = false;
                    currentTurnPlayer = nextPlayer;

                    if (bidder >= 0) {
                        if (amount > 0) {
                            currentHighBid = amount;
                            currentHighBidder = bidder;
                            lastActionMsg = room.playerNames[bidder] + " bid $" + std::to_string(amount);
                        } else {
                            lastActionMsg = room.playerNames[bidder] + " passed";

                            int revealedId = -1;
                            if (revealedCount < (int)gameData.CardsPlayed.size()) {
                                revealedId = gameData.CardsPlayed[revealedCount];
                            }

                            if (revealedId != -1 && revealedCount < nPlayers) {
                                TableSlot& slot = slots[revealedCount];
                                slot.revealed = true;
                                
                                int realIndex = std::abs(revealedId);
                                if (realIndex > 9) realIndex = 2; // 防呆
                                
                                sf::Vector2f pos = slot.widget.rect.getPosition();
                                sf::Vector2f size = slot.widget.rect.getSize();
                                slot.widget.init(font, realIndex, pos, size);
                                
                                revealedCount++;
                            }
                        }
                    }

                    if (room.inGame % 2 != 0) {
                        if (currentHighBidder == -1 && bidder >= 0) currentHighBidder = bidder;

                        if (currentHighBidder != -1) {
                            if (currentHighBidder >= 0 && currentHighBidder < nPlayers) {
                                lastActionMsg = room.playerNames[currentHighBidder] + " won the bid";
                            } else {
                                lastActionMsg = "Unknown player won the bid";
                            }
                            if (currentHighBidder == myIndex) wonRoundsCount++;
                        } else {
                            lastActionMsg = "Round Ended (No Winner)";
                        }

                        showingResult = true;
                        resultTimer.restart();

                        // 翻開所有牌 (Reveal All)
                        for(int i = 0; i < nPlayers && i < (int)gameData.CardsPlayed.size(); ++i) {
                            TableSlot& slot = slots[i];
                            int cId = gameData.CardsPlayed[i];
                            slot.revealed = true;
                            
                            int realIndex = std::abs(cId);
                            if (realIndex > 9) realIndex = 2;

                            sf::Vector2f pos = slot.widget.rect.getPosition();
                            sf::Vector2f size = slot.widget.rect.getSize();
                            slot.widget.init(font, realIndex, pos, size);
                        }

                        bidPanel.setVisible(false);
                        hasSubmitted = false;
                        
                        // 狀態改變，跳出迴圈
                        break;
                    }
                }
            }
        }

        // ---------------------------------------------------
        //  2. UI 邏輯
        // ---------------------------------------------------
        bool isMyTurn = (currentTurnPlayer == myIndex);

        if (showingResult) {
            float elapsed = resultTimer.getElapsedTime().asSeconds();
            int remaining = (int)(SHOW_DURATION - elapsed);
            
            if (remaining > 0) {
                std::string msg = lastActionMsg + "\nNext Round in " + std::to_string(remaining) + "s";
                updateBroadcast(msg);
                timerLabel.set(""); 
            } else {
                showingResult = false;
                
                // 檢查是否遊戲完全結束 (第 10 輪，inGame >= 19)
                // 注意：這裡假設 gameData.Score() 會在 main.cpp 切換狀態前被呼叫，
                // 或者我們在這裡呼叫它。
                if (gameData.myRoom.inGame >= 19) {
                     gameData.Score(); // 接收 ws 封包
                     state = State::Settlement; // 切換到結算畫面
                     return;
                }

                currentHighBidder = -1;
                isBiddingPhase = false;
                playedCount = 0;
                currentHighBid = 0;
                revealedCount = 0;

                for (auto& s : slots) {
                    s.filled = false;
                    s.revealed = false;
                    s.widget.rect.setFillColor(sf::Color(50, 50, 50));
                    s.widget.text.setString("");
                    int amount = makeupMoney[&s - &slots[0]]; // 計算 index
                    if (amount > 0) s.moneyLabel.set("+" + std::to_string(amount));
                    
                    // 重新置中
                    sf::FloatRect b = s.moneyLabel.text.getLocalBounds();
                    s.moneyLabel.text.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
                    sf::Vector2f pos = s.widget.rect.getPosition();
                    sf::Vector2f size = s.widget.rect.getSize();
                    s.moneyLabel.text.setPosition(pos.x + size.x/2.f, pos.y - 20.f);
                }
                lastActionMsg = "New Round Start"; 
                updateBroadcast(lastActionMsg);

                std::string newTitle = "Play Phase - Round " + std::to_string(gameData.Round());
                titleLabel.set(newTitle);
                titleLabel.centerText();
            }
        }
        else {
            // ... (Bidding UI Logic) ...
            if (isBiddingPhase) {
                if (isMyTurn && !hasSubmitted) {
                    bidPanel.setVisible(true);
                    int minBid = (currentHighBid == 0) ? 1 : currentHighBid + 1;
                    bidPanel.setRange(minBid, gameData.Money());
                    updateBroadcast(lastActionMsg + "\nYour turn to Bid");
                } else {
                    bidPanel.setVisible(false);
                    updateBroadcast(lastActionMsg);
                }
            }
            else {
                bidPanel.setVisible(false);
                updateBroadcast(isMyTurn ? "Your turn play card" : lastActionMsg);
            }

            if (isMyTurn) {
                int remain = (isBiddingPhase ? 60 : 45) - (int)stepTimer.getElapsedTime().asSeconds();
                if (remain < 0) remain = 0;
                timerLabel.set("Time left - " + std::to_string(remain) + " s");
                timerLabel.text.setFillColor(remain <= 10 ? sf::Color::Red : sf::Color::White);
                timerLabel.centerText();
            } else {
                timerLabel.set("");
            }

            moneyLabel.set("Money: $" + std::to_string(gameData.Money()));
            {
                sf::FloatRect b = moneyLabel.text.getLocalBounds();
                moneyLabel.text.setOrigin(b.left, b.top + b.height/2.f); 
                moneyLabel.text.setPosition(60.f, 550.f);
            }
        }

        // ---------------------------------------------------
        //  3. 事件
        // ---------------------------------------------------
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed) window.close();
            if (e.type == sf::Event::Resized) updateBackgroundUI();

            if (!showingResult) 
            {
                if (!isBiddingPhase) 
                {
                    hand.handleClick(e, window);
                    if (playBtn.clicked(e, window) && isMyTurn) {
                        int cardId = hand.selectedCardId();
                        if (cardId != -1) {
                            if (gameData.Play(cardId)) {
                                playBtn.setDisabled(true);
                                lastActionMsg = "You played card " + std::to_string(cardId);
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
                    auto action = bidPanel.handleEvent(e, window);
                    if (action == BidPanel::Action::Submit) {
                        gameData.SendBid(bidPanel.value);
                        bidPanel.setVisible(false);
                        hasSubmitted = true;
                    }
                    else if (action == BidPanel::Action::Pass) {
                        gameData.SendBid(0);
                        bidPanel.setVisible(false);
                        hasSubmitted = true;
                    }
                }
            }
        }

        if (!isBiddingPhase && !showingResult) {
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

        titleLabel.draw(window);
        
        for (auto& s : slots){
            s.widget.draw(window);
            if (isBiddingPhase && !s.revealed) {
                s.moneyLabel.draw(window);
            }
        }
        
        hand.draw(window); 

        if (!showingResult) {
            if (!isBiddingPhase) playBtn.draw(window);
            if (isBiddingPhase) bidPanel.draw(window);
        }

        broadcastLabel.draw(window);
        if (isMyTurn) timerLabel.draw(window);
        moneyLabel.draw(window);
        
        window.draw(wonStackBg); 

        if (wonRoundsCount > 0) {
            float baseX = 660.f + (120.f - 70.f)/2.f; 
            float baseY = 460.f; 
            wonCardRect.setPosition(baseX , baseY);
            window.draw(wonCardRect);

            countLabel.set("x " + std::to_string(wonRoundsCount));
            countLabel.text.setPosition(baseX + 20.f, baseY - 50.f); 
            countLabel.draw(window);
        }

        sf::RectangleShape border = wonStackBg;
        border.setFillColor(sf::Color::Transparent);
        window.draw(border);

        window.draw(wonStackFrame);
        wonLabel.draw(window);

        window.display();
    }
}