#include "ui/pages/rules_page.hpp"
#include "ui/common/ui_common.hpp"
#include "ui/common/ui_background.hpp"

#include <vector>
#include <string>

extern sf::View uiView;

// ============================================================
// UI constants
// ============================================================
namespace RulesUI {
    constexpr float WIDTH  = 800.f;
    constexpr float HEIGHT = 600.f;

    // Panel
    constexpr float PANEL_W = 600.f;
    constexpr float PANEL_H = 400.f;
    constexpr float PANEL_X = (WIDTH - PANEL_W) / 2.f;
    constexpr float PANEL_Y = (HEIGHT - PANEL_H) / 2.f;

    // Tabs
    constexpr float TAB_H = 50.f;
    constexpr float TAB_W = PANEL_W / 2.f;

    // Scroll area
    constexpr float SCROLL_X = PANEL_X + 20.f;
    constexpr float SCROLL_Y = PANEL_Y + TAB_H + 20.f;
    constexpr float SCROLL_W = PANEL_W - 40.f;
    constexpr float SCROLL_H = PANEL_H - TAB_H - 80.f;

    // Buttons
    constexpr float BACK_W = 160.f;
    constexpr float BACK_H = 60.f;
    constexpr float BACK_X = WIDTH / 2.f;
    constexpr float BACK_Y = PANEL_Y + PANEL_H - 30.f;
}

// ============================================================
// ScrollArea class
// ============================================================
class ScrollArea {
public:
    std::vector<sf::Text> lines;
    float offsetY = 0.f;
    float totalHeight = 0.f;
    float viewHeight = 0.f;
    sf::Font* font;

    float x, y, w, h;

    ScrollArea(float px, float py, float pw, float ph, sf::Font* ft)
        : x(px), y(py), w(pw), h(ph), font(ft)
    {
        viewHeight = ph;
    }

    void setText(const std::string& all, int charSize = 22)
    {
        lines.clear();
        offsetY = 0.f;

        float curY = 0.f;

        std::string temp;
        for (char c : all) {
            temp.push_back(c);
            if (c == '\n') {
                sf::Text tx(temp, *font, charSize);
                tx.setFillColor(sf::Color::Black);
                tx.setPosition(x, y + curY);
                lines.push_back(tx);
                curY += charSize + 8;
                temp.clear();
            }
        }
        if (!temp.empty()) {
            sf::Text tx(temp, *font, charSize);
            tx.setFillColor(sf::Color::Black);
            tx.setPosition(x, y + curY);
            lines.push_back(tx);
            curY += charSize + 8;
        }

        totalHeight = curY;
    }

    void handleScroll(const sf::Event& e)
    {
        if (e.type == sf::Event::MouseWheelScrolled) {
            offsetY -= e.mouseWheelScroll.delta * 40.f;

            float maxOffset = std::max(0.f, totalHeight - viewHeight);
            if (offsetY < 0) offsetY = 0;
            if (offsetY > maxOffset) offsetY = maxOffset;
        }
    }

    void draw(sf::RenderWindow& window)
    {
        // Clip using view
        sf::View old = window.getView();

        sf::View clip(sf::FloatRect(x, y, w, h));
        clip.setViewport(sf::FloatRect(
            x / RulesUI::WIDTH,
            y / RulesUI::HEIGHT,
            w / RulesUI::WIDTH,
            h / RulesUI::HEIGHT
        ));
        window.setView(clip);

        for (auto& t : lines) {
            sf::Text temp = t;
            temp.move(0, -offsetY);
            window.draw(temp);
        }

        window.setView(old);
    }
};

// ============================================================
// Dummy text (you will replace later)
// ============================================================
static const char* GAME_RULES_TEXT =
"【Game Rules】\n"
"1. 每位玩家會獲得 5 張卡。\n"
"2. 玩家依序進行競標。\n"
"3. 出價最高者獲得該輪卡片。\n"
"4. 所有卡片結算後，最高分者勝利。\n"
"\n"
"(後續你可以補上真正的遊戲規則)";

static const char* GAME_LIMITS_TEXT =
"【Game Limits】\n"
"1. 30 秒未 Ready，玩家會被踢出房間。\n"
"2. 私人房間最多 2 間。\n"
"3. Room 滿 5 人會自動 Lock。\n"
"\n"
"(後續你可以補上真正的限制細節)";


// ============================================================
// Main function
// ============================================================
void runRulesPage(sf::RenderWindow& window, State& state)
{
    using namespace RulesUI;

    sf::Font font;
    loadFontSafe(font);
    

    // Panel
    sf::RectangleShape panel({PANEL_W, PANEL_H});
    panel.setPosition(PANEL_X, PANEL_Y);
    panel.setFillColor(sf::Color(255,255,255,240));
    panel.setOutlineColor(sf::Color(150,150,150));
    panel.setOutlineThickness(4);

    // Tabs
    Button rulesTab(&font, "Game Rules", PANEL_X + TAB_W/2.f, PANEL_Y + TAB_H/2.f, TAB_W, TAB_H, true);
    Button limitsTab(&font, "Game Limits", PANEL_X + TAB_W + TAB_W/2.f, PANEL_Y + TAB_H/2.f, TAB_W, TAB_H, true);

    bool showingRules = true; // 預設顯示 Game Rules

    // Scroll area
    ScrollArea scroller(SCROLL_X, SCROLL_Y, SCROLL_W, SCROLL_H, &font);
    scroller.setText(GAME_RULES_TEXT);

    // Back button
    Button backBtn(&font, "BACK", BACK_X, BACK_Y, BACK_W, BACK_H, true);

    // ------------------------------------------------------------
    while (window.isOpen() && state == State::Rules)
    {

        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed)
                window.close();

            scroller.handleScroll(e);

            if (rulesTab.clicked(e, window)) {
                showingRules = true;
                scroller.setText(GAME_RULES_TEXT);
            }

            if (limitsTab.clicked(e, window)) {
                showingRules = false;
                scroller.setText(GAME_LIMITS_TEXT);
            }

            if (backBtn.clicked(e, window)) {
                state = State::UsernameInput;
                return;
            }
        }

        // Hover
        rulesTab.update(window);
        limitsTab.update(window);
        backBtn.update(window);

        if (showingRules) {
            rulesTab.shape.setFillColor(sf::Color(255,240,150));
            limitsTab.shape.setFillColor(sf::Color(220,220,220));
        } else {
            limitsTab.shape.setFillColor(sf::Color(255,240,150));
            rulesTab.shape.setFillColor(sf::Color(220,220,220));
        }

        // ------------------------------------------------------------
        // Draw
        window.setView(uiView);
        window.clear();
        drawBackground(window);

        window.draw(panel);

        rulesTab.draw(window);
        limitsTab.draw(window);
        scroller.draw(window);

        backBtn.draw(window);

        window.display();
    }
}
