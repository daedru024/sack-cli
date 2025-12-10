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
    // 2. Define Content Text (Translated to English)
    // ---------------------------------------------------------
    
    // Tab 1: Game Mechanics (Updated based on your code logic)
    std::string rulesText = 
        "[ OBJECTIVE ]\n"
        "   Earn the most points after 9 Rounds.\n"
        "   Final Score = Remaining Cash + Value of Won Cards.\n\n"

        "[ CARDS ]\n"
        "   Cats:      -8, -5, 3, 5, 8, 11, 15 points.\n"
        "   Rabbit:    0 points (Neutral).\n\n"

        "[ THE DOGS ]\n"
        "   Dogs eliminate a card in the current stack.\n"
        "   Big Dog (-9):  Removes the MAX value card.\n"
        "   Small Dog (9): Removes the MIN value card.\n"
        "   * The dog itself counts as -9 or 9 points unless\n"
        "     canceling out another card.\n\n"

        "[ PASSING COMPENSATION ]\n"
        "   If you PASS, you get cash based on order:\n"
        "   3 Players: 1st(+$3), 2nd(+$6)\n"
        "   4 Players: 1st(+$2), 2nd(+$4), 3rd(+$6)\n"
        "   5 Players: 1st(+$2), 2nd(+$3), 3rd(+$4), 4th(+$6)\n"
        "   * You also reveal one hidden card on the table.\n\n"

        "[ WINNING A ROUND ]\n"
        "   The last player remaining MUST pay their bid.\n"
        "   They take all cards in the stack.";

    // Tab 2: System Limits & Controls
    std::string limitsText = 
        "[ INITIAL SETUP ]\n"
        "   Cash:      Starts with $15.\n"
        "   Hand:      10 Cards initially.\n"
        "   Discard:   1 Card is removed before game starts.\n\n"

        "[ TIME LIMITS ]\n"
        "   Action:    60 seconds per turn.\n"
        "   Results:   10 seconds result display per round.\n"
        "   Lobby:     Auto-kick after 30s idle.\n\n"

        "[ CONTROLS ]\n"
        "   Play:      Select card -> Click 'PLAY CARD'.\n"
        "   Bid:       Input amount -> Click 'SUBMIT'.\n"
        "   Pass:      Click 'PASS' (Collect compensation).\n\n"
        
        "[ RESTRICTIONS ]\n"
        "   - You cannot bid more than your current cash.\n"
        "   - Once passed, you are out for this round.\n"
        "   - Played cards cannot be taken back.";

    // ---------------------------------------------------------
    // 3. UI Initialization
    // ---------------------------------------------------------

    // Title
    Label titleLabel(&font, "How to Play", 400.f, 40.f, 48, sf::Color::Yellow);
    titleLabel.text.setOutlineColor(sf::Color::Black);
    titleLabel.text.setOutlineThickness(2.f);
    titleLabel.centerText();

    // Tab Buttons
    float tabY = 100.f;
    Button rulesTab(&font, "Rules", 200.f, tabY, 180.f, 50.f, true);
    Button limitsTab(&font, "Limits", 420.f, tabY, 180.f, 50.f, true);
    
    // Back Button (Exit)
    constexpr float EXIT_W = 120.f;
    constexpr float EXIT_H = 40.f;
    Button backBtn(&font, "Back", 720.f, 40.f, 120.f, 40.f, true);

    // Content Labels
    // Position set to (0,0) as they are drawn inside the View
    Label labelRules(&font, rulesText, 0.f, 0.f, 24, sf::Color::White);
    Label labelLimits(&font, limitsText, 0.f, 0.f, 24, sf::Color::White);
    
    // Line spacing adjustment
    labelRules.text.setLineSpacing(1.3f);
    labelLimits.text.setLineSpacing(1.3f);

    // ---------------------------------------------------------
    // 4. Scroll View Settings
    // ---------------------------------------------------------
    
    // Define view area on screen (x, y, width, height)
    sf::FloatRect viewRect(100.f, 180.f, 600.f, 340.f);
    
    // Create View
    sf::View contentView;
    contentView.setSize(viewRect.width, viewRect.height);
    contentView.setCenter(viewRect.width / 2.f, viewRect.height / 2.f);

    // Set Viewport (0.0 ~ 1.0 relative to window)
    float vpX = viewRect.left / 800.f;
    float vpY = viewRect.top / 600.f;
    float vpW = viewRect.width / 800.f;
    float vpH = viewRect.height / 600.f;
    contentView.setViewport(sf::FloatRect(vpX, vpY, vpW, vpH));

    // ---------------------------------------------------------
    // 5. State Variables
    // ---------------------------------------------------------
    
    bool showRules = true; // true = Rules, false = Limits
    float scrollY = 0.f;   // Current Y scroll offset

    // ---------------------------------------------------------
    // 6. Main Loop
    // ---------------------------------------------------------
    while (window.isOpen() && state == State::Rules)
    {
        // Calculate max scroll
        float contentHeight = showRules ? labelRules.text.getLocalBounds().height 
                                        : labelLimits.text.getLocalBounds().height;
        float maxScroll = std::max(0.f, contentHeight - viewRect.height + 50.f); 

        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed) window.close();
            if (e.type == sf::Event::Resized) updateBackgroundUI();

            // --- Scroll Handling ---
            if (e.type == sf::Event::MouseWheelScrolled) {
                float delta = -e.mouseWheelScroll.delta * 30.f; 
                scrollY += delta;
            }

            // --- Button Interaction ---
            backBtn.update(window);
            rulesTab.update(window);
            limitsTab.update(window);

            if (backBtn.clicked(e, window)) {
                state = State::UsernameInput; 
            }

            // Tab Switching
            if (rulesTab.clicked(e, window)) {
                showRules = true;
                scrollY = 0.f;
            }
            if (limitsTab.clicked(e, window)) {
                showRules = false;
                scrollY = 0.f;
            }
        }

        // --- Keyboard Scroll ---
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) scrollY -= 5.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) scrollY += 5.f;

        // --- Clamp Scroll ---
        if (scrollY < 0.f) scrollY = 0.f;
        if (scrollY > maxScroll) scrollY = maxScroll;

        // --- Update Tab State ---
        rulesTab.setDisabled(showRules);
        limitsTab.setDisabled(!showRules);

        // ---------------------------------------------------------
        // 7. Drawing
        // ---------------------------------------------------------
        window.clear();
        drawBackground(window);

        // A. Draw Fixed UI
        titleLabel.draw(window);
        rulesTab.draw(window);
        limitsTab.draw(window);
        backBtn.draw(window);

        // Optional Border for content area
        sf::RectangleShape border(sf::Vector2f(viewRect.width + 10, viewRect.height + 10));
        border.setPosition(viewRect.left - 5, viewRect.top - 5);
        border.setFillColor(sf::Color(0, 0, 0, 150)); // Semi-transparent black background
        border.setOutlineColor(sf::Color::White);
        border.setOutlineThickness(2);
        window.draw(border);

        // B. Draw Scrollable Content
        float defaultCenterX = viewRect.width / 2.f;
        float defaultCenterY = viewRect.height / 2.f;
        contentView.setCenter(defaultCenterX, defaultCenterY + scrollY);
        
        window.setView(contentView);

        if (showRules) {
            labelRules.draw(window);
        } else {
            labelLimits.draw(window);
        }

        // Reset View
        window.setView(window.getDefaultView());

        window.display();
    }
}