#include "ui/pages/rules_page.hpp"
#include "ui/common/ui_common.hpp"
#include "ui/common/ui_background.hpp"
#include "ui/widgets/label.hpp"
#include "ui/widgets/button.hpp"

#include <algorithm> // for std::max, std::min
#include <iostream>  // for error logging

void runRulesPage(sf::RenderWindow& window, State& state)
{
    // 1. Load Font
    sf::Font font;
    loadFontSafe(font);

    // ---------------------------------------------------------
    // Load Dog Images
    // ---------------------------------------------------------
    sf::Texture bigDogTexture;
    if (!bigDogTexture.loadFromFile("assets/card_8.png")) {
        std::cout << "Error loading card_8.png" << std::endl;
    }
    sf::Sprite bigDogSprite(bigDogTexture);

    sf::Texture smallDogTexture;
    if (!smallDogTexture.loadFromFile("assets/card_9.png")) {
        std::cout << "Error loading card_9.png" << std::endl;
    }
    sf::Sprite smallDogSprite(smallDogTexture);
    
    float targetHeight = 50.f;
    
    // 自動計算縮放比例 (Scale = 目標高度 / 原圖高度)
    if (bigDogTexture.getSize().y > 0) {
        float scale = targetHeight / bigDogTexture.getSize().y;
        bigDogSprite.setScale(scale, scale);
    }
    if (smallDogTexture.getSize().y > 0) {
        float scale = targetHeight / smallDogTexture.getSize().y;
        smallDogSprite.setScale(scale, scale);
    }

    // ---------------------------------------------------------
    // 2. Define Content Text 
    // ---------------------------------------------------------

    std::string rulesText = 
        "[ OBJECTIVE ]\n"
        "   Earn the most points after 9 Rounds.\n"
        "   Final Score = Remaining Cash + Value of Won Cards.\n\n"
        
        "[ INITIAL SETUP ]\n"
        "   $15 cash and 10 cards.\n"
        "   1 card will be removed by the player before you.\n\n"
        
        "[ CARDS ]\n"
        "   Each cat values -8, -5, 3, 5, 8, 11 or 15 points.\n"
        "   The rabbit has 0 points in value.\n\n"
        
        "[ THE DOGS ]\n"
        "   • 1 dog chases away at most 1 cat.\n"
        "   • The staring dog                   chases away the cat with the\n"
        "         maximum positive value.\n"
        "   • The sleeping dog                  chases away the cat with the\n"
        "         minimum negative value.\n"
        "   *   If more than 2 dogs appear in one stack, all dogs\n"
        "          are removed without taking away any cat.\n\n"
        
        "[ GAME FLOW ]\n"
        "   1.Discard  ->  2.Play Cards  ->  3.Auction  ->  4.Back to 2.\n"
        "   • Playing order:\n"
        "        The first player to play a card is called the start\n"
        "        player. After each round, the start player becomes\n"
        "        the last player, and the second player becomes\n"
        "        the start player, and so on.\n\n"

        "[ PLAYING CARDS ]\n"
        "   • Choose one card to play each round.\n"
        "   • Played cards are put face down on the table,\n"
        "        arranged left-to-right by play order.\n"
        "   • Once played, it cannot be taken back.\n\n"
        
        "[ AUCTION ]\n"
        "   • The start player bids first. When it's your turn, you\n"
        "        must make a higher bid than the current price.\n"
        "   • You must pass if you don't have enough money.\n"
        "   • Passing makes you unable to bid again for this\n"
        "        round. You will take back the money you bid and\n"
        "        also earn some compensation.\n"
        "   • When all other players passed, the remaining\n"
        "        player wins the stack if they made a bid.\n"
        "   • The winner must pay their bid and take the whole\n"
        "        stack.\n"
        "   • If every player passed, nobody wins.\n\n"

        "[ 3-PLAYER MODE ]\n"
        "   There will be a BOT playing as the 4th player.";

    std::string limitsText = 
        "[ TIME LIMITS ]\n"
        "   Lobby:       60s to join a room.\n"
        "   Room:       30s to ready up.\n"
        "   Discard:    30s to discard another player's card.\n"
        "   Play/Bid:  60s to play or bid.\n\n"

        "[ ROOM LIMITS ]\n"
        "   Password: 3 wrong attempts allowed.\n\n"

        "[ AUTO PLAYER ]\n"
        "   Replaces the player who left the game.\n"
        "   If multiple players left the game, gameplay ends.\n"
        "   * This feature does not support 3-player mode.\n\n"

        "[ CONTROLS ]\n"
        "   Join Room:      Click room -> Password -> 'JOIN'.\n"
        "   Ready:                Choose color -> Click 'READY'.\n"
        "   Start(HOST):  Click 'LOCK&START'.\n"
        "   Discard:             Select card -> Click 'CONFIRM'.\n"
        "   Play:                    Select card -> Click 'PLAY CARD'.\n"
        "   Bid:                      Adjust price -> Click 'OK'.\n"
        "   Pass:                   Click 'PASS' (Get compensation).\n"
        "   Rules:                 Click 'RULES' to read this.\n"
        "   Exit:                     Click 'EXIT' to leave.";

    // ---------------------------------------------------------
    // 3. UI Initialization & View Settings
    // ---------------------------------------------------------
    sf::FloatRect viewRect(100.f, 180.f, 600.f, 340.f);

    Label titleLabel(&font, "How to Play", 400.f, 40.f, 48, sf::Color::Yellow);
    titleLabel.text.setOutlineColor(sf::Color::Black);
    titleLabel.text.setOutlineThickness(2.f);
    titleLabel.centerText();

    float tabY = 100.f;
    Button rulesTab(&font, "Rules", 200.f, tabY, 180.f, 50.f, true);
    Button limitsTab(&font, "Limits", 420.f, tabY, 180.f, 50.f, true);
    Button backBtn(&font, "Back", 720.f, 40.f, 120.f, 40.f, true);

    Label labelRules(&font, rulesText, 0.f, 0.f, 24, sf::Color::White);
    Label labelLimits(&font, limitsText, 0.f, 0.f, 24, sf::Color::White);
    
    labelRules.text.setLineSpacing(1.4f);
    labelLimits.text.setLineSpacing(1.4f);

    sf::RectangleShape scrollBarTrack;
    scrollBarTrack.setFillColor(sf::Color(50, 50, 50));
    scrollBarTrack.setSize(sf::Vector2f(viewRect.width, 12.f)); 
    scrollBarTrack.setPosition(viewRect.left, viewRect.top + viewRect.height + 5.f); 

    sf::RectangleShape scrollBarThumb;
    scrollBarThumb.setFillColor(sf::Color(150, 150, 150));
    scrollBarThumb.setSize(sf::Vector2f(0.f, 12.f)); 
    scrollBarThumb.setPosition(viewRect.left, viewRect.top + viewRect.height + 5.f);

    // ---------------------------------------------------------
    // 4. View Logic
    // ---------------------------------------------------------
    sf::View contentView;
    contentView.setSize(viewRect.width, viewRect.height);
    contentView.setCenter(viewRect.width / 2.f, viewRect.height / 2.f);

    float vpX = viewRect.left / 800.f;
    float vpY = viewRect.top / 600.f;
    float vpW = viewRect.width / 800.f;
    float vpH = viewRect.height / 600.f;
    contentView.setViewport(sf::FloatRect(vpX, vpY, vpW, vpH));

    // ---------------------------------------------------------
    // 5. State Variables
    // ---------------------------------------------------------
    bool showRules = true;
    float scrollY = 0.f;
    float scrollX = 0.f; 

    bool isDraggingScrollbar = false;
    float dragOffsetX = 0.f;

    // ---------------------------------------------------------
    // 6. Main Loop
    // ---------------------------------------------------------
    while (window.isOpen() && state == State::Rules)
    {
        sf::FloatRect contentBounds = showRules ? labelRules.text.getLocalBounds() 
                                                : labelLimits.text.getLocalBounds();
        float contentHeight = contentBounds.height;
        float contentWidth = contentBounds.width;
        float maxScrollY = std::max(0.f, contentHeight - viewRect.height + 50.f);
        float maxScrollX = std::max(0.f, contentWidth - viewRect.width + 50.f);
        float trackWidth = scrollBarTrack.getSize().x;
        float ratio = viewRect.width / (contentWidth + 50.f);
        if (ratio > 1.f) ratio = 1.f;
        float thumbWidth = trackWidth * ratio;
        if (thumbWidth < 30.f) thumbWidth = 30.f; 
        scrollBarThumb.setSize(sf::Vector2f(thumbWidth, 12.f));

        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed) window.close();
            if (e.type == sf::Event::Resized) updateBackgroundUI();
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f worldMousePos = window.mapPixelToCoords(mousePos);
            if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
            {
                if (maxScrollX > 0 && scrollBarThumb.getGlobalBounds().contains(worldMousePos))
                {
                    isDraggingScrollbar = true;
                    dragOffsetX = worldMousePos.x - scrollBarThumb.getPosition().x;
                }
            }
            if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left)
            {
                isDraggingScrollbar = false;
            }
            if (e.type == sf::Event::MouseMoved && isDraggingScrollbar)
            {
                float trackLeft = scrollBarTrack.getPosition().x;
                float trackRightLimit = trackLeft + trackWidth - thumbWidth;
                float newThumbX = worldMousePos.x - dragOffsetX;
                if (newThumbX < trackLeft) newThumbX = trackLeft;
                if (newThumbX > trackRightLimit) newThumbX = trackRightLimit;
                float progress = (newThumbX - trackLeft) / (trackWidth - thumbWidth);
                scrollX = progress * maxScrollX;
            }
            if (e.type == sf::Event::MouseWheelScrolled) {
                float delta = -e.mouseWheelScroll.delta * 30.f; 
                if (e.mouseWheelScroll.wheel == sf::Mouse::HorizontalWheel || 
                   (e.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel && sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))) 
                {
                    scrollX += delta;
                }
                else 
                {
                    scrollY += delta;
                }
            }
            backBtn.update(window);
            rulesTab.update(window);
            limitsTab.update(window);
            if (backBtn.clicked(e, window)) {
                state = State::UsernameInput; 
            }
            if (rulesTab.clicked(e, window)) {
                showRules = true;
                scrollY = 0.f;
                scrollX = 0.f;
            }
            if (limitsTab.clicked(e, window)) {
                showRules = false;
                scrollY = 0.f;
                scrollX = 0.f;
            }
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) scrollY -= 5.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) scrollY += 5.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) scrollX -= 5.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) scrollX += 5.f;
        if (scrollY < 0.f) scrollY = 0.f;
        if (scrollY > maxScrollY) scrollY = maxScrollY;
        if (scrollX < 0.f) scrollX = 0.f;
        if (scrollX > maxScrollX) scrollX = maxScrollX;
        if (maxScrollX > 0)
        {
            float scrollProgress = scrollX / maxScrollX;
            float trackLeft = scrollBarTrack.getPosition().x;
            float trackWidth = scrollBarTrack.getSize().x;
            float currentThumbX = trackLeft + (scrollProgress * (trackWidth - thumbWidth));
            scrollBarThumb.setPosition(currentThumbX, scrollBarThumb.getPosition().y);
        }
        rulesTab.setDisabled(showRules);
        limitsTab.setDisabled(!showRules);

        // ---------------------------------------------------------
        // 7. Drawing
        // ---------------------------------------------------------
        window.clear();
        drawBackground(window);

        // A. Fixed UI
        titleLabel.draw(window);
        rulesTab.draw(window);
        limitsTab.draw(window);
        backBtn.draw(window);

        sf::RectangleShape border(sf::Vector2f(viewRect.width + 10, viewRect.height + 10));
        border.setPosition(viewRect.left - 5, viewRect.top - 5);
        border.setFillColor(sf::Color(0, 0, 0, 150));
        border.setOutlineColor(sf::Color::White);
        border.setOutlineThickness(2);
        window.draw(border);

        // B. Scrollable Content
        float defaultCenterX = viewRect.width / 2.f;
        float defaultCenterY = viewRect.height / 2.f;
        contentView.setCenter(defaultCenterX + scrollX, defaultCenterY + scrollY);
        
        window.setView(contentView);

        if (showRules) {
            labelRules.draw(window);

            // ---------------------------------------------------------
            // Position and Draw Dog Sprites using findCharacterPos
            // ---------------------------------------------------------
            
            // 搜尋文字 "The staring dog" 在整段字串中的位置
            std::string searchStr1 = "The staring dog";
            std::size_t index1 = rulesText.find(searchStr1);
            
            if (index1 != std::string::npos) {
                // 取得這段文字 "結尾處" 的座標 (x, y)
                // 取 searchStr1.length() 代表要在這段字後面開始畫
                sf::Vector2f pos = labelRules.text.findCharacterPos(index1 + searchStr1.length());
                
                // 設定圖片位置
                // X: 文字結尾往右移 10 pixel
                // Y: 文字基準線(Baseline)往上移一些，讓圖片垂直置中
                bigDogSprite.setPosition(pos.x + 10.f, pos.y - 15.f);
                window.draw(bigDogSprite);
            }

            // 定位 Small Dog (Sleeping Dog)
            std::string searchStr2 = "The sleeping dog";
            std::size_t index2 = rulesText.find(searchStr2);
            
            if (index2 != std::string::npos) {
                sf::Vector2f pos = labelRules.text.findCharacterPos(index2 + searchStr2.length());
                
                smallDogSprite.setPosition(pos.x + 10.f, pos.y - 15.f);
                window.draw(smallDogSprite);
            }

        } else {
            labelLimits.draw(window);
        }

        // Reset View for Fixed UI (Scrollbar is fixed UI)
        window.setView(window.getDefaultView());

        // C. Draw Scrollbar (Only if scrollable)
        if (maxScrollX > 0) {
            window.draw(scrollBarTrack);
            window.draw(scrollBarThumb);
        }

        window.display();
    }
}