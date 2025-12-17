#include "ui/pages/rules_page.hpp"
#include "ui/common/ui_common.hpp"
#include "ui/common/ui_background.hpp"
#include "ui/widgets/label.hpp"
#include "ui/widgets/button.hpp"

#include <algorithm> // for std::max, std::min

void runRulesPage(sf::RenderWindow& window, State& state)
{
    // 1. Load Font
    sf::Font font;
    loadFontSafe(font);

    // ---------------------------------------------------------
    // 2. Define Content Text 
    // ---------------------------------------------------------

    std::string rulesText = 
        "[ OBJECTIVE ]\n"
        "   Earn the most points after 9 Rounds.\n"
        "   Final Score = Remaining Cash + Value of Won Cards.\n\n"
        
        "[ GAME FLOW ]\n"
        "   Discard -> Round Start -> Play Cards -> Bidding -> New Round\n"
        "   * Game ends when all players have emptied their hands.\n\n"
        
        "[ INITIAL SETUP ]\n"
        "   Cash:       Starts with $15.\n"
        "   Hand:       10 Cards initially.\n"
        "   Discard:    1 Card removed by opponent before start.\n\n"
        
        "[ CARDS ]\n"
        "   Cats:       -8, -5, 3, 5, 8, 11, 15 points.\n"
        "   Rabbit:     0 points.\n\n"
        
        "[ THE DOGS ]\n"
        "   Dogs eliminate a card in the current stack.\n"
        "   Big Dog:    Removes the MAX positive value card.\n"
        "   Small Dog:  Removes the MIN value card.\n"
        "   * If both appear, both get removed.\n\n"
        
        "[ PLAYING CARDS ]\n"
        "   • Choose one card to play each round.\n"
        "   • Played cards are hidden on the table.\n"
        "   • Cards are arranged left-to-right by play order.\n"
        "   • Round 1: Host starts.\n"
        "     Round 2: 2nd player to join starts (and so on).\n\n"
        
        "[ BIDDING ]\n"
        "   • The first player to play a card bids first.\n"
        "   • Initial bid is $1.\n"
        "   • Turn:     PASS or BID higher than current max.\n"
        "   • Winner:   The last player remaining wins the stack.\n"
        "   • All Pass: Cards are discarded, nobody wins.\n\n"
        
        "[ PASSING COMPENSATION ]\n"
        "   If you PASS, you earn cash based on order:\n"
        "   3 Players:  Same as 4 players.\n"
        "   4 Players:  1st(+$2), 2nd(+$4), 3rd(+$6).\n"
        "   5 Players:  1st(+$2), 2nd(+$3), 3rd(+$4), 4th(+$6).\n"
        "   * Leftmost hidden card is revealed upon passing.\n\n"

        "[ WINNING A ROUND ]\n"
        "   • Bidding winner MUST pay their bid amount.\n"
        "   • Winner takes all cards in the stack.\n\n"

        "[ RESTRICTIONS ]\n"
        "   • Bid cannot exceed current cash.\n"
        "   • Once passed, you are out for the round.\n"
        "   • Played cards cannot be taken back.\n\n"

        "[ NOTIFICATIONS ]\n"
        "   There will be a BOT playing in 3-players mode.";

    std::string limitsText = 
        "[ TIME LIMITS ]\n"
        "   Action:       60s (Any interaction resets timer).\n"
        "   Lobby:        60s idle leads to auto-kick.\n"
        "   Room:         30s to ready up.\n"
        "   Discard:      45s to discard initial card.\n"
        "   Play/Bid:     60s to play or bid.\n\n"

        "[ ROOM LIMITS ]\n"
        "   Password:     3 attempts allowed max.\n\n"

        "[ AUTO PLAYER ]\n"
        "   Replaces you if kicked by timeout.\n"
        "   3 Players:    Not allowed (Game Ends).\n"
        "   4/5 Players:  Max 1 Auto-Player allowed.\n\n"

        "[ CONTROLS ]\n"
        "   Join Room:    Click room -> Password -> 'JOIN'.\n"
        "   Ready:        Choose color -> Click 'READY'.\n"
        "   Start(HOST):  Click 'LOCK&START'.\n"
        "   Discard:      Select card -> Click 'CONFIRM'.\n"
        "   Play:         Select card -> Click 'PLAY CARD'.\n"
        "   Bid:          Use Arrow Buttons -> Click 'OK'.\n"
        "   Pass:         Click 'PASS' (Get compensation).\n"
        "   Rules:        Click 'RULES' to read this.\n"
        "   Exit:         Click 'EXIT' to leave.";

    // ---------------------------------------------------------
    // 3. UI Initialization & View Settings
    // ---------------------------------------------------------

    // [MOVED UP] Define View Area first to use in Scrollbar setup
    sf::FloatRect viewRect(100.f, 180.f, 600.f, 340.f);

    // Title
    Label titleLabel(&font, "How to Play", 400.f, 40.f, 48, sf::Color::Yellow);
    titleLabel.text.setOutlineColor(sf::Color::Black);
    titleLabel.text.setOutlineThickness(2.f);
    titleLabel.centerText();

    // Tab Buttons
    float tabY = 100.f;
    Button rulesTab(&font, "Rules", 200.f, tabY, 180.f, 50.f, true);
    Button limitsTab(&font, "Limits", 420.f, tabY, 180.f, 50.f, true);
    
    // Back Button
    Button backBtn(&font, "Back", 720.f, 40.f, 120.f, 40.f, true);

    // Content Labels (Position at 0,0 for View)
    Label labelRules(&font, rulesText, 0.f, 0.f, 24, sf::Color::White);
    Label labelLimits(&font, limitsText, 0.f, 0.f, 24, sf::Color::White);
    
    labelRules.text.setLineSpacing(1.4f);
    labelLimits.text.setLineSpacing(1.4f);

    // Horizontal Scrollbar UI 
    sf::RectangleShape scrollBarTrack;
    scrollBarTrack.setFillColor(sf::Color(50, 50, 50));
    scrollBarTrack.setSize(sf::Vector2f(viewRect.width, 12.f)); 
    // Position below the content view
    scrollBarTrack.setPosition(viewRect.left, viewRect.top + viewRect.height + 5.f); 

    sf::RectangleShape scrollBarThumb;
    scrollBarThumb.setFillColor(sf::Color(150, 150, 150));
    scrollBarThumb.setSize(sf::Vector2f(0.f, 12.f)); // Width will be dynamic
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

    // Dragging State
    bool isDraggingScrollbar = false;
    float dragOffsetX = 0.f;

    // ---------------------------------------------------------
    // 6. Main Loop
    // ---------------------------------------------------------
    while (window.isOpen() && state == State::Rules)
    {
        // --- 1. Calculate Content Size & Bounds ---
        sf::FloatRect contentBounds = showRules ? labelRules.text.getLocalBounds() 
                                                : labelLimits.text.getLocalBounds();
        
        float contentHeight = contentBounds.height;
        float contentWidth = contentBounds.width;

        float maxScrollY = std::max(0.f, contentHeight - viewRect.height + 50.f);
        float maxScrollX = std::max(0.f, contentWidth - viewRect.width + 50.f);

        // --- 2. Update Scrollbar Thumb Size (Dynamic) ---
        float trackWidth = scrollBarTrack.getSize().x;
        // Ratio = Visible / Total. If Total < Visible, ratio = 1 (full width)
        float ratio = viewRect.width / (contentWidth + 50.f);
        if (ratio > 1.f) ratio = 1.f;

        float thumbWidth = trackWidth * ratio;
        if (thumbWidth < 30.f) thumbWidth = 30.f; // Minimum width for usability
        scrollBarThumb.setSize(sf::Vector2f(thumbWidth, 12.f));

        // --- 3. Event Processing ---
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed) window.close();
            if (e.type == sf::Event::Resized) updateBackgroundUI();

            // Mouse Position for Dragging
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f worldMousePos = window.mapPixelToCoords(mousePos);

            // Handle Drag Start
            if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
            {
                // Only allow dragging if there is content to scroll
                if (maxScrollX > 0 && scrollBarThumb.getGlobalBounds().contains(worldMousePos))
                {
                    isDraggingScrollbar = true;
                    dragOffsetX = worldMousePos.x - scrollBarThumb.getPosition().x;
                }
            }

            // Handle Drag End
            if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left)
            {
                isDraggingScrollbar = false;
            }

            // Handle Dragging Movement
            if (e.type == sf::Event::MouseMoved && isDraggingScrollbar)
            {
                float trackLeft = scrollBarTrack.getPosition().x;
                float trackRightLimit = trackLeft + trackWidth - thumbWidth;
                
                // Calculate new thumb X based on mouse minus offset
                float newThumbX = worldMousePos.x - dragOffsetX;

                // Clamp to track
                if (newThumbX < trackLeft) newThumbX = trackLeft;
                if (newThumbX > trackRightLimit) newThumbX = trackRightLimit;

                // Map Thumb Position -> ScrollX
                // Progress (0.0 to 1.0)
                float progress = (newThumbX - trackLeft) / (trackWidth - thumbWidth);
                scrollX = progress * maxScrollX;
            }

            // Mouse Wheel (Supports Horizontal & Vertical)
            if (e.type == sf::Event::MouseWheelScrolled) {
                float delta = -e.mouseWheelScroll.delta * 30.f; 

                // Check for Horizontal Wheel OR Shift + Vertical Wheel
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

            // Buttons
            backBtn.update(window);
            rulesTab.update(window);
            limitsTab.update(window);

            if (backBtn.clicked(e, window)) {
                state = State::UsernameInput; 
            }

            // Tab Switching (Reset scrolls)
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

        // --- 4. Keyboard Scrolling (X & Y) ---
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) scrollY -= 5.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) scrollY += 5.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) scrollX -= 5.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) scrollX += 5.f;

        // --- 5. Clamp Scroll Values ---
        if (scrollY < 0.f) scrollY = 0.f;
        if (scrollY > maxScrollY) scrollY = maxScrollY;

        if (scrollX < 0.f) scrollX = 0.f;
        if (scrollX > maxScrollX) scrollX = maxScrollX;

        // --- 6. Sync Thumb Position (if not dragging) ---
        // This ensures keyboard/wheel scrolling updates the UI bar
        if (maxScrollX > 0)
        {
            float scrollProgress = scrollX / maxScrollX;
            float trackLeft = scrollBarTrack.getPosition().x;
            float trackWidth = scrollBarTrack.getSize().x;
            
            // Map ScrollX -> Thumb Position
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

        // Border
        sf::RectangleShape border(sf::Vector2f(viewRect.width + 10, viewRect.height + 10));
        border.setPosition(viewRect.left - 5, viewRect.top - 5);
        border.setFillColor(sf::Color(0, 0, 0, 150));
        border.setOutlineColor(sf::Color::White);
        border.setOutlineThickness(2);
        window.draw(border);

        // B. Scrollable Content
        // Apply both X and Y offsets to View Center
        float defaultCenterX = viewRect.width / 2.f;
        float defaultCenterY = viewRect.height / 2.f;
        contentView.setCenter(defaultCenterX + scrollX, defaultCenterY + scrollY);
        
        window.setView(contentView);

        if (showRules) {
            labelRules.draw(window);
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