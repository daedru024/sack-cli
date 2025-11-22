#include "ui/pages/username_page.hpp"

// extern GamePlay gameData;
// extern const char servip[10];

void runUsernamePage(
    sf::RenderWindow& window,
    State& state,
    std::string& username,
    EndReason& reason
){
    sf::Font font;
    loadFontSafe(font);

    Label title(&font, "Cat In The Sack", 180, 70, 56,
                sf::Color::White, sf::Color::Black, 4);

    sf::RectangleShape panel({500, 300});
    panel.setFillColor(sf::Color(255,255,255,220));
    panel.setOutlineColor(sf::Color(100,100,100));
    panel.setOutlineThickness(4);
    panel.setPosition(150, 150);

    Label enterUser(&font, "Enter Username", 260, 160, 32,
                    sf::Color::White, sf::Color::Black, 3);

    TextBox usernameBox(&font, 250, 230, 300, 55);
    Button okBtn(&font, "START", 310, 330, 180, 65);
    Button exitBtn(&font, "Exit", 650, 20, 120, 40);

    while (window.isOpen() && state == State::UsernameInput)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            usernameBox.handleEvent(event, window);

            // START
            if (okBtn.clicked(event, window))
            {
                username = usernameBox.buffer;
                if (!username.empty())
                {
                    // ★ 完全符合原始 main.cpp 的寫法
                    gameData = GamePlay(servip.c_str(), username);
                    state = State::RoomInfo;
                }
            }

            // Exit
            if (exitBtn.clicked(event, window))
            {
                reason = EndReason::UserExit;
                state = State::EndConn;
            }
        }

        okBtn.update(window);
        exitBtn.update(window);

        window.setView(uiView);
        window.clear();

        drawBackground(window);
        window.draw(panel);

        title.draw(window);
        enterUser.draw(window);
        usernameBox.draw(window);
        okBtn.draw(window);
        exitBtn.draw(window);

        window.display();
    }
}
