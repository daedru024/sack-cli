#include "ui/pages/endconn_page.hpp"
#include "ui/common/ui_common.hpp"
#include "ui/common/ui_background.hpp"

extern sf::View uiView;
extern sf::Sprite& g_bgSprite();
extern sf::RectangleShape& g_bgOverlay();
extern bool UI_TEST_MODE;


namespace EndUI {
    constexpr float WIDTH   = 800.f;
    constexpr float HEIGHT  = 600.f;

    constexpr float PANEL_W = 520.f;
    constexpr float PANEL_H = 260.f;
    constexpr float PANEL_X = (WIDTH - PANEL_W) / 2.f;
    constexpr float PANEL_Y = (HEIGHT - PANEL_H) / 2.f;

    constexpr float TITLE_Y = PANEL_Y + 40.f;

    constexpr float MSG_Y   = PANEL_Y + 130.f;

    constexpr float BTN_W = 160.f;
    constexpr float BTN_H = 60.f;
    constexpr float BTN_X = WIDTH / 2.f;
    constexpr float BTN_Y = PANEL_Y + PANEL_H - 50.f;
}

using namespace EndUI;

void runEndConnPage(sf::RenderWindow& window, State& state, EndReason reason)
{
    sf::Font font;
    loadFontSafe(font);

    // ---- Panel ----
    sf::RectangleShape panel({PANEL_W, PANEL_H});
    panel.setFillColor(sf::Color(255,255,255,240));
    panel.setOutlineColor(sf::Color(180,180,180));
    panel.setOutlineThickness(3.f);
    panel.setPosition(PANEL_X, PANEL_Y);

    // ---- Message ----
    std::string msg;
    switch (reason) {
        case EndReason::RoomsFull:
            msg = "All rooms are full.\nConnection closed."; break;
        case EndReason::UserExit:
            msg = "You exited the lobby.\nConnection closed."; break;
        case EndReason::WrongKeyTooMany:
            msg = "Too many wrong passwords.\nConnection closed."; break;
        case EndReason::Timeout:
            msg = "Timeout: You did not join a room in time.\nConnection closed."; break;
        default:
            msg = "Connection ended."; break;
    }

    // ---- Title ----
    Label title(&font, "Disconnected",
        WIDTH / 2.f, TITLE_Y, 40,
        sf::Color::White, sf::Color::Black, 4);
    title.centerText();

    // ---- OK Button ----
    Button okBtn(
        &font, "OK",
        BTN_X, BTN_Y,
        BTN_W, BTN_H,
        true
    );

    // ----------------- Main Loop -----------------
    while (window.isOpen() && state == State::EndConn)
    {
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed)
                window.close();

            if (okBtn.clicked(e, window)) {
                // 回到初始輸入頁面
                state = State::UsernameInput;
                return;
            }
        }

        okBtn.update(window);

        // ---- Draw ----
        window.setView(uiView);
        window.clear();

        window.draw(g_bgSprite());
        window.draw(g_bgOverlay());

        window.draw(panel);
        title.draw(window);

        // ---- 多行文字置中 ----
        sf::Text tx = makeMultilineCenterText(font, msg, 24, sf::Color::Black);
        tx.setPosition(WIDTH / 2.f, MSG_Y);
        window.draw(tx);

        okBtn.draw(window);

        window.display();
    }
}
