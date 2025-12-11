#include "ui/pages/settlement_page.hpp"
#include "ui/common/ui_common.hpp"
#include "ui/common/ui_background.hpp"
#include "libcliwrap.hpp"
#include "room.hpp"

// 引入 Widget
#include "ui/widgets/card_widget.hpp"
#include "ui/widgets/label.hpp"
#include "ui/widgets/game_cards.hpp"

#include <vector>
#include <string>
#include <cmath> // for std::abs

// 引用全域變數
extern sf::View   uiView;
extern GamePlay   gameData;

void runSettlementPage(
    sf::RenderWindow& window,
    State&            state,
    const std::string& username)
{
    sf::Font font;
    loadFontSafe(font);

    Room& room = gameData.myRoom;
    int nPlayers = room.n_players;

    // =================================================================================
    // 1. UI 元件初始化 - 卡牌與玩家區域
    // =================================================================================

    struct PlayerSlot {
        CardWidget card;
        Label      nameLabel;
        
        PlayerSlot(const sf::Font* f) 
            : nameLabel(f, "", 0, 0, 24, sf::Color::White, sf::Color::Black, 2.f) 
        {}
    };
    
    std::vector<PlayerSlot> slots;
    slots.reserve(nPlayers);
    for(int i=0; i<nPlayers; ++i) slots.emplace_back(&font);

    // --- 版面配置 ---
    float slotWidth = 80.f;
    float slotHeight = 110.f;
    float slotGap = 40.f; 
    float totalWidth = nPlayers * slotWidth + (nPlayers - 1) * slotGap;
    float startX = (800.f - totalWidth) / 2.f;
    float cardY = 240.f; 

    for (int i = 0; i < nPlayers; i++) {
        sf::Vector2f pos(startX + i * (slotWidth + slotGap), cardY);
        sf::Vector2f size(slotWidth, slotHeight);

        slots[i].card.init(font, 2, pos, size); // 預設兔子/牌背

        slots[i].nameLabel.set(room.playerNames[i]);
        sf::FloatRect b = slots[i].nameLabel.text.getLocalBounds();
        slots[i].nameLabel.text.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
        slots[i].nameLabel.text.setPosition(pos.x + slotWidth/2.f, pos.y - 25.f);
    }

    // =================================================================================
    // 2. [每回合結算] UI 元素
    // =================================================================================

    // 標題：金色 + 粗黑邊
    Label roundTitle(&font, "Round 1", 400, 60, 64, sf::Color(255, 215, 0), sf::Color::Black, 4.f);
    roundTitle.centerText();

    // 倒數計時器：更顯眼 (白色帶黑邊，加大)
    Label timerLabel(&font, "Next Round: 5", 780, 40, 28, sf::Color::White, sf::Color::Black, 2.f);
    // 靠右對齊
    {
        sf::FloatRect b = timerLabel.text.getLocalBounds();
        timerLabel.text.setOrigin(b.left + b.width, b.top + b.height/2.f);
        timerLabel.text.setPosition(780, 40);
    }

    // 底部資訊面板 (半透明黑底圓角框)
    sf::RectangleShape infoPanel;
    infoPanel.setSize({420.f, 130.f});
    infoPanel.setOrigin(210.f, 65.f);
    infoPanel.setPosition(400.f, 480.f);
    infoPanel.setFillColor(sf::Color(0, 0, 0, 200)); // 深黑，透明度較低
    infoPanel.setOutlineColor(sf::Color(150, 150, 150)); // 灰色邊框
    infoPanel.setOutlineThickness(2.f);

    // 贏家文字 (金色)
    Label winnerText(&font, "Winner: ???", 400, 450, 36, sf::Color(255, 223, 0)); 
    winnerText.text.setOutlineColor(sf::Color::Black);
    winnerText.text.setOutlineThickness(1.f);
    winnerText.centerText();

    // 分數文字 (青色)
    Label stackScoreText(&font, "Stack Score: 0", 400, 500, 32, sf::Color(0, 255, 255));
    stackScoreText.text.setOutlineColor(sf::Color::Black);
    stackScoreText.text.setOutlineThickness(1.f);
    stackScoreText.centerText();

    // =================================================================================
    // 3. [最終結算] UI 元素
    // =================================================================================
    
    // 最終分數的大背景板
    sf::RectangleShape finalPanel;
    finalPanel.setSize({550.f, 450.f});
    finalPanel.setOrigin(275.f, 225.f);
    finalPanel.setPosition(400.f, 300.f);
    finalPanel.setFillColor(sf::Color(20, 20, 20, 230));
    finalPanel.setOutlineColor(sf::Color::White);
    finalPanel.setOutlineThickness(3.f);

    // 標題 (綠色帶黑邊)
    Label finalTitle(&font, "FINAL SCORES", 400, 140, 60, sf::Color(50, 205, 50), sf::Color::Black, 3.f);
    finalTitle.centerText();

    // 提示文字
    Label exitHint(&font, "Press any key to return...", 400, 500, 22, sf::Color(220, 220, 220), sf::Color::Black, 1.f);
    exitHint.centerText();

    struct FinalRow {
        Label rank;
        Label name;
        Label score;
    };
    std::vector<FinalRow> finalRows;

    // =================================================================================
    // 4. 狀態與迴圈
    // =================================================================================
    sf::Clock roundTimer;
    sf::Clock finalTimer;
    int currentDisplayRound = 0; 
    bool showFinal = false;

    while (window.isOpen() && state == State::Settlement)
    {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) window.close();
            
            if (showFinal && (e.type == sf::Event::KeyPressed || e.type == sf::Event::MouseButtonPressed)) {
                state = State::RoomInfo; 
                return;
            }
        }

        // --- 邏輯更新 ---
        float dt = roundTimer.getElapsedTime().asSeconds();

        if (!showFinal) {
            int remaining = 5 - (int)dt;
            if (remaining < 0) remaining = 0;
            
            // 更新倒數文字
            timerLabel.set("Next Round: " + std::to_string(remaining));
            // 重新靠右對齊
            sf::FloatRect b = timerLabel.text.getLocalBounds();
            timerLabel.text.setOrigin(b.left + b.width, b.top + b.height/2.f);
            timerLabel.text.setPosition(780, 40);


            if (dt >= 5.0f) {
                currentDisplayRound++;
                roundTimer.restart();

                if (currentDisplayRound >= 9) {
                    showFinal = true;
                    finalTimer.restart();
                    
                    // 1. 建立配對 (分數, 名字)
                    std::vector<std::pair<int, std::string>> rankedScores;
                    for(int i=0; i<nPlayers; ++i) {
                        rankedScores.push_back({gameData.Results.PlayerScore[i], room.playerNames[i]});
                    }
                    
                    // 2. 排序 (分數高到低)
                    std::sort(rankedScores.begin(), rankedScores.end(), [](const auto& a, const auto& b) {
                        return a.first > b.first;
                    });

                    // 3. 建立 Label
                    float startY = 220.f;
                    float gapY = 55.f;
                    for(int i=0; i<nPlayers; ++i) {
                        std::string rankStr;
                        sf::Color rankColor;

                        // 設定排名文字與顏色
                        if (i == 0) { rankStr = "1st"; rankColor = sf::Color(255, 215, 0); }      // 金
                        else if (i == 1) { rankStr = "2nd"; rankColor = sf::Color(192, 192, 192); } // 銀
                        else if (i == 2) { rankStr = "3rd"; rankColor = sf::Color(205, 127, 50); }  // 銅
                        else { rankStr = std::to_string(i+1) + "th"; rankColor = sf::Color::White; }

                        // 建立三個 Label
                        // Rank (靠右對齊 x=320)
                        Label lRank(&font, rankStr, 0, 0, 36, rankColor, sf::Color::Black, 1.5f);
                        sf::FloatRect rb = lRank.text.getLocalBounds();
                        lRank.text.setOrigin(rb.left + rb.width, rb.top + rb.height/2.f);
                        lRank.text.setPosition(320.f, startY + i * gapY);

                        // Name (靠左對齊 x=350)
                        Label lName(&font, rankedScores[i].second, 0, 0, 36, sf::Color::White, sf::Color::Black, 1.5f);
                        sf::FloatRect nb = lName.text.getLocalBounds();
                        lName.text.setOrigin(nb.left, nb.top + nb.height/2.f);
                        lName.text.setPosition(350.f, startY + i * gapY);

                        // Score (靠右對齊 x=550) - 使用青色區分
                        Label lScore(&font, std::to_string(rankedScores[i].first), 0, 0, 36, sf::Color(0, 255, 255), sf::Color::Black, 1.5f);
                        sf::FloatRect sb = lScore.text.getLocalBounds();
                        lScore.text.setOrigin(sb.left + sb.width, sb.top + sb.height/2.f);
                        lScore.text.setPosition(550.f, startY + i * gapY);

                        finalRows.push_back({lRank, lName, lScore});
                    }
                }
            }
        }
        else {
            // === 最終分數階段邏輯 (Auto-Kick) ===
            float finalDt = finalTimer.getElapsedTime().asSeconds();
            int kickRemain = 30 - (int)finalDt;

            if (kickRemain <= 0) {
                state = State::RoomInfo;
                return;
            }

            // 更新提示文字
            exitHint.set("Returning to lobby in " + std::to_string(kickRemain) + "s... (or press any key)");
            exitHint.centerText();
        }

        // --- 繪圖 ---
        window.setView(uiView);
        window.clear();
        drawBackground(window); 

        if (!showFinal) {
            // === [回合展示階段] ===
            
            roundTitle.set("Round " + std::to_string(currentDisplayRound + 1));
            roundTitle.centerText();
            roundTitle.draw(window);

            // 更新並繪製卡牌
            if (currentDisplayRound < (int)gameData.Results.stks.size()) {
                const auto& currentStack = gameData.Results.stks[currentDisplayRound];
                for (int i = 0; i < nPlayers; i++) {
                    int pID = (i + currentDisplayRound) % nPlayers;

                    slots[i].nameLabel.set(room.playerNames[pID]);
                   
                    sf::FloatRect b = slots[i].nameLabel.text.getLocalBounds();
                    slots[i].nameLabel.text.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);

                    sf::Vector2f slotPos = slots[i].card.rect.getPosition();
                    slots[i].nameLabel.text.setPosition(slotPos.x + 80.f/2.f, slotPos.y - 25.f);


                    //int cardID = (pID < (int)currentStack.size()) ? currentStack[pID] : 2; 
                    int cardID = (i < (int)currentStack.size()) ? currentStack[i] : 2; 

                    int realIndex = std::abs(cardID);
                    if (realIndex > 9) realIndex = 2;

                    sf::Vector2f size = slots[i].card.rect.getSize();
                    slots[i].card.init(font, realIndex, slotPos, size);

                    slots[i].card.draw(window);
                    slots[i].nameLabel.draw(window);
                }
            }

            window.draw(infoPanel);

            if (currentDisplayRound < (int)gameData.Results.winner.size()) {
                int wID = gameData.Results.winner[currentDisplayRound];
                int sVal = gameData.Results.stackValue[currentDisplayRound];

                std::string wName = "No One";
                if (wID >= 0 && wID < nPlayers) {
                    wName = room.playerNames[wID];
                }
                
                winnerText.set("Winner: " + wName);
                winnerText.centerText();
                winnerText.draw(window);
                
                stackScoreText.set("Stack Score: " + std::to_string(sVal));
                stackScoreText.centerText();
                stackScoreText.draw(window);
            }

            timerLabel.draw(window);
        }
        else {
            // === [最終分數階段] ===
            window.draw(finalPanel);
            finalTitle.draw(window);
            for(auto& row : finalRows) {
                row.rank.draw(window);
                row.name.draw(window);
                row.score.draw(window);
            }
            exitHint.draw(window);
        }

        window.display();
    }
}