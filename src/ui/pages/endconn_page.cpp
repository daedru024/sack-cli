#include "ui/pages/endconn_page.hpp"
// #include "ui/common/ui_background.hpp"

void runEndConnPage(sf::RenderWindow& window, State& state, EndReason reason)
{
    sf::Font font;
    font.loadFromFile("fonts/NotoSans-Regular.ttf");

    std::string msg;
    switch (reason) {
        case EndReason::RoomsFull:        msg = "All rooms are full.\nConnection closed."; break;
        case EndReason::UserExit:         msg = "You exited the lobby.\nConnection closed."; break;
        case EndReason::WrongKeyTooMany:  msg = "Too many wrong passwords.\nConnection closed."; break;
        case EndReason::Timeout:          msg = "Timeout: You did not join a room in 60s.\nConnection closed."; break;
        default:                          msg = "Connection ended."; break;
    }

    Button okBtn(&font, "OK", 330, 350, 160, 60);

    while (window.isOpen() && state == State::EndConn)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (okBtn.clicked(event, window))
                window.close();
        }

        okBtn.update(window);

        window.setView(uiView);
        window.clear();

        window.draw(g_bgSprite());
        window.draw(g_bgOverlay());

        sf::RectangleShape panel({520, 260});
        panel.setPosition(140, 170);
        panel.setFillColor(sf::Color(255,255,255,240));
        panel.setOutlineColor(sf::Color(180,180,180));
        panel.setOutlineThickness(3);
        window.draw(panel);

        Label title(&font, "Disconnected", 240, 190, 40,
                    sf::Color::White, sf::Color::Black, 4);
        title.draw(window);

        sf::Text tx(msg, font, 24);
        tx.setFillColor(sf::Color::Black);
        tx.setPosition(170, 240);
        window.draw(tx);

        okBtn.draw(window);
        window.display();
    }
}
