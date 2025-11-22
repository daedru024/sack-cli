#include "ui/pages/host_setting_page.hpp"

void runHostSettingPage(
    sf::RenderWindow& window,
    State& state,
    EndReason& /*reason*/,
    Room& room,
    const std::string& username
){
    sf::Font font;
    font.loadFromFile("fonts/NotoSans-Regular.ttf");

    Label title(&font, "Room Privacy", 240, 50, 46,
                sf::Color::White, sf::Color::Black, 4);

    Label hostLabel(&font, "Host: " + username, 240, 110, 24,
                    sf::Color::White, sf::Color::Black, 2);

    Button publicBtn (&font, "Public", 190, 180, 180, 60);
    Button privateBtn(&font, "Private", 410, 180, 180, 60);

    publicBtn.text.setCharacterSize(28);
    privateBtn.text.setCharacterSize(28);
    centerTextInButton(publicBtn.text, publicBtn.shape);
    centerTextInButton(privateBtn.text, privateBtn.shape);

    bool isPrivate = room.isPrivate;
    std::string pwBuf = room.password;

    const int PASS_LEN = 4;
    sf::RectangleShape pwBox[PASS_LEN];
    for (int i = 0; i < PASS_LEN; i++) {
        pwBox[i].setSize({50, 60});
        pwBox[i].setOutlineThickness(3);
        pwBox[i].setOutlineColor(sf::Color::Black);
        pwBox[i].setFillColor(sf::Color(255,255,255,230));
    }

    Label pwLabel(&font, "Password (4 digits)", 230, 260, 22,
                  sf::Color::White, sf::Color::Black, 2);

    Button confirmBtn(&font, "CONFIRM", 300, 420, 200, 70);
    confirmBtn.text.setCharacterSize(28);
    centerTextInButton(confirmBtn.text, confirmBtn.shape);

    std::string errorMsg;

    while (window.isOpen() && state == State::HostSetting)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (publicBtn.clicked(event, window)) {
                isPrivate = false;
                pwBuf.clear();
                errorMsg.clear();
            }
            if (privateBtn.clicked(event, window)) {
                isPrivate = true;
                errorMsg.clear();
            }

            // 密碼輸入
            if (isPrivate && event.type == sf::Event::TextEntered)
            {
                if (event.text.unicode == 8) {   // backspace
                    if (!pwBuf.empty())
                        pwBuf.pop_back();
                }
                else if (event.text.unicode >= '0' &&
                         event.text.unicode <= '9')
                {
                    if (pwBuf.size() < PASS_LEN)
                        pwBuf.push_back((char)event.text.unicode);
                }
            }

            // 確認
            if (confirmBtn.clicked(event, window))
            {
                if (isPrivate)
                {
                    if (pwBuf.size() != 4) {
                        errorMsg = "Password must be 4 digits.";
                        continue;
                    }

                    // main.cpp 行為：最多 2 間 private
                    // int cnt = countPrivateRooms(&room);
                    // if (cnt >= 2) {
                    //     errorMsg = "At most 2 PRIVATE rooms allowed.";
                    //     isPrivate = false;
                    //     pwBuf.clear();
                    //     continue;
                    // }
                }

                // 寫入房間設定
                room.isPrivate = isPrivate;
                room.password  = isPrivate ? pwBuf : "";

                state = State::InRoom;
                return;
            }
        }

        publicBtn.update(window);
        privateBtn.update(window);
        confirmBtn.update(window);

        window.setView(uiView);
        window.clear();
        drawBackground(window);

        title.draw(window);
        hostLabel.draw(window);

        // Public / Private highlight
        publicBtn.shape.setFillColor(!isPrivate ? sf::Color(200,240,200)
                                                : sf::Color(220,220,220));
        privateBtn.shape.setFillColor(isPrivate ? sf::Color(200,240,200)
                                                : sf::Color(220,220,220));

        publicBtn.draw(window);
        privateBtn.draw(window);

        // 密碼 UI
        if (isPrivate)
        {
            pwLabel.draw(window);

            for (int i = 0; i < PASS_LEN; i++)
            {
                pwBox[i].setPosition(230 + i * 60.f, 300);
                window.draw(pwBox[i]);

                if (i < pwBuf.size()) {
                    sf::Text digit(std::string(1, pwBuf[i]), font, 32);
                    digit.setFillColor(sf::Color::Black);
                    digit.setPosition(240 + i * 60.f, 298);
                    window.draw(digit);
                }
            }
        }

        if (!errorMsg.empty()) {
            sf::Text e(errorMsg, font, 20);
            e.setFillColor(sf::Color::Red);
            e.setPosition(230, 380);
            window.draw(e);
        }

        confirmBtn.draw(window);
        window.display();
    }
}
