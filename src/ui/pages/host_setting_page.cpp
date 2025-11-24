#include "ui/pages/host_setting_page.hpp"
#include "ui/common/ui_background.hpp"
#include "ui/common/ui_common.hpp"
#include "libcliwrap.hpp"

extern sf::View uiView;
extern GamePlay gameData;
extern bool UI_TEST_MODE;


using std::string;

namespace HostUI {
    constexpr float WIDTH  = 800.f;
    constexpr float HEIGHT = 600.f;
    constexpr float CENTER_X = WIDTH / 2.f;

    // Title / Host Name
    constexpr float TITLE_Y      = 60.f;
    constexpr float HOST_LABEL_Y = 120.f;

    // Public / Private buttons
    constexpr float BTN_Y        = 190.f;
    constexpr float BTN_W        = 220.f;
    constexpr float BTN_H        = 60.f;
    constexpr float BTN_GAP      = 40.f;

    // Password input
    constexpr int   PASS_LEN     = 4;
    constexpr float PASS_W       = 50.f;
    constexpr float PASS_H       = 60.f;
    constexpr float PASS_GAP     = 20.f;

    constexpr float PASS_LABEL_Y = 280.f;
    constexpr float PASS_BOX_Y   = 315.f;
    constexpr float ERROR_MSG_Y  = PASS_BOX_Y + 90.f;

    // Confirm button
    constexpr float CONFIRM_Y    = 430.f;
    constexpr float CONFIRM_W    = 240.f;
    constexpr float CONFIRM_H    = 70.f;
}


// ─────────────────────────────────────────────
// Main
// ─────────────────────────────────────────────
void runHostSettingPage(
    sf::RenderWindow& window,
    State& state,
    EndReason&,
    Room& room,
    const std::string& username)
{
    using namespace HostUI;

    sf::Font font;
    loadFontSafe(font);

    // Title
    Label title(&font, "Room Privacy",
                CENTER_X, TITLE_Y, 48,
                sf::Color::White, sf::Color::Black, 4);
    title.centerText();

    Label hostLabel(&font, "Host: " + username,
                    CENTER_X, HOST_LABEL_Y, 24,
                    sf::Color::White, sf::Color::Black, 2);
    hostLabel.centerText();

    // Buttons
    Button publicBtn(&font, "Public",
                     CENTER_X - BTN_W/2.f - BTN_GAP/2.f, BTN_Y,
                     BTN_W, BTN_H, true);

    Button privateBtn(&font, "Private",
                      CENTER_X + BTN_W/2.f + BTN_GAP/2.f, BTN_Y,
                      BTN_W, BTN_H, true);

    bool isPrivate = room.isPrivate;

    // Password label
    Label pwLabel(&font, "Password (4 digits)",
                  CENTER_X, PASS_LABEL_Y, 22,
                  sf::Color::Black, sf::Color::Transparent, 0);
    pwLabel.centerText();

    sf::RectangleShape pwBox[PASS_LEN];
    for (int i = 0; i < PASS_LEN; i++) {
        pwBox[i].setSize({PASS_W, PASS_H});
        pwBox[i].setFillColor(sf::Color(255,255,255,230));
        pwBox[i].setOutlineColor(sf::Color::Black);
        pwBox[i].setOutlineThickness(3);
    }

    string pwBuf = room.password;
    string errorMsg;

    Button confirmBtn(&font, "CONFIRM",
                      CENTER_X, CONFIRM_Y,
                      CONFIRM_W, CONFIRM_H, true);

    // ───────────── MAIN LOOP ─────────────
    while (window.isOpen() && state == State::HostSetting)
    {
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed) window.close();

            // toggle privacy
            if (publicBtn.clicked(e, window)) {
                isPrivate = false;
                pwBuf.clear();
                errorMsg.clear();
            }
            if (privateBtn.clicked(e, window)) {
                isPrivate = true;
                errorMsg.clear();
            }

            // password input
            if (isPrivate && e.type == sf::Event::TextEntered)
            {
                if (e.text.unicode == 8) { // backspace
                    if (!pwBuf.empty()) pwBuf.pop_back();
                }
                else if (e.text.unicode >= '0' && e.text.unicode <= '9')
                {
                    if ((int)pwBuf.size() < PASS_LEN)
                        pwBuf.push_back((char)e.text.unicode);
                }

                if (!errorMsg.empty()) errorMsg.clear();
            }

            bool wantConfirm =
                confirmBtn.clicked(e, window) ||
                (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Enter);

            if (wantConfirm)
            {

                // set Private
                if (isPrivate)
                {
                    if (pwBuf.size() != PASS_LEN)
                    {
                        errorMsg = "PIN must be 4 digits.";
                        continue;
                    }

                    int er = gameData.MakePrivate(pwBuf);
                    if (er != SUCCESS) {
                        errorMsg = "Server refused PIN.";
                        continue;
                    }
                }
                else
                {
                    // set Public
                    int er = gameData.UnlockRoom();
                    if (er != SUCCESS) {
                        errorMsg = "Server rejected public setting.";
                        continue;
                    }
                }

                // local update
                room.isPrivate = isPrivate;
                room.password = isPrivate ? pwBuf : "";

                state = State::InRoom;
                return;
            }
        }

        // update UI hover
        publicBtn.update(window);
        privateBtn.update(window);
        confirmBtn.update(window);

        // draw
        window.setView(uiView);
        window.clear();
        drawBackground(window);

        title.draw(window);
        hostLabel.draw(window);

        // Highlight public/private buttons
        publicBtn.shape.setFillColor(
            !isPrivate ? sf::Color(180,230,180) : sf::Color(210,210,210)
        );
        privateBtn.shape.setFillColor(
            isPrivate ? sf::Color(180,230,180) : sf::Color(210,210,210)
        );

        publicBtn.draw(window);
        privateBtn.draw(window);

        // Password UI
        if (isPrivate)
        {
            pwLabel.draw(window);

            float totalW = PASS_LEN * PASS_W + (PASS_LEN - 1) * PASS_GAP;
            float startX = CENTER_X - totalW/2.f;

            for (int i = 0; i < PASS_LEN; i++)
            {
                pwBox[i].setPosition(startX + i*(PASS_W + PASS_GAP), PASS_BOX_Y);

                bool focus = (i == (int)pwBuf.size());
                pwBox[i].setOutlineColor(focus ? sf::Color(0,150,255)
                                               : sf::Color::Black);
                pwBox[i].setOutlineThickness(focus ? 4.f : 3.f);

                window.draw(pwBox[i]);

                // draw digit
                if (i < (int)pwBuf.size())
                {
                    sf::Text d = mkCenterText(font, string(1, pwBuf[i]),
                                              32, sf::Color::Black);
                    d.setPosition(
                        pwBox[i].getPosition().x + PASS_W/2.f,
                        PASS_BOX_Y + PASS_H/2.f
                    );
                    window.draw(d);
                }
            }
        }

        // error message
        if (!errorMsg.empty())
        {
            sf::Text eText = mkCenterText(font, errorMsg, 20, sf::Color::Red);
            eText.setPosition(CENTER_X, ERROR_MSG_Y);
            window.draw(eText);
        }

        confirmBtn.draw(window);
        window.display();
    }
}
