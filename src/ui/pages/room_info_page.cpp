#include "ui/pages/room_info_page.hpp"
#include "ui/common/ui_background.hpp"
#include "ui/common/ui_common.hpp"

#include <algorithm>
#include <iostream>

extern std::vector<Room> rooms;
extern GamePlay gameData;
extern int currentRoomIndex;

void runRoomInfoPage(
    sf::RenderWindow& window,
    State& state,
    EndReason& reason,
    const std::string& username
){
    sf::Font font;
    loadFontSafe(font);

    Label title(&font, "Select a Room", 200, 40, 50,
                sf::Color::White, sf::Color::Black, 4);

    Button exitBtn(&font, "Exit", 650, 20, 120, 40);

    Button roomBtn[3] = {
        Button(&font, "Room 1", 120, 140, 360, 80),
        Button(&font, "Room 2", 120, 260, 360, 80),
        Button(&font, "Room 3", 120, 380, 360, 80)
    };

    const int PASS_LEN = 4;
    std::string keyInput;

    sf::RectangleShape passBox[PASS_LEN];
    for (int i = 0; i < PASS_LEN; i++) {
        passBox[i].setSize({40, 50});
        passBox[i].setFillColor(sf::Color(255,255,255,230));
        passBox[i].setOutlineColor(sf::Color::Black);
        passBox[i].setOutlineThickness(3);
    }

    Label passLabel(&font, "Password (4 digits)", 520, 205, 22,
                    sf::Color::White, sf::Color::Black, 2);

    Button joinBtn(&font, "JOIN", 520, 360, 150, 60);

    Label selectedLabel(&font, "", 500, 150, 24,
                        sf::Color::White, sf::Color::Black, 3);

    int  selected      = -1;
    bool needsKey      = false;
    std::string errorMsg;
    int  wrongKeyCount = 0;

    sf::Clock timer;


    while (window.isOpen() && state == State::RoomInfo)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            // EXIT
            if (exitBtn.clicked(event, window)) {
                reason = EndReason::UserExit;
                state = State::EndConn;
                return;
            }

            auto canClick = [&](int i) {
                Room& r = rooms[i];
                if (r.inGame) return false;
                if (r.isFull()) return false;
                return true;
            };

            // ===== Select room =====
            for (int i = 0; i < 3; i++)
            {
                if (canClick(i) && roomBtn[i].clicked(event, window))
                {
                    selected = i;
                    Room& r = rooms[i];

                    needsKey = (r.isPrivate && !r.playerNames.empty());
                    keyInput.clear();
                    errorMsg.clear();

                    selectedLabel.text.setString("Selected: " + r.name);
                }
            }

            // ===== Password typing =====
            if (needsKey && selected != -1 &&
                event.type == sf::Event::TextEntered)
            {
                if (event.text.unicode >= '0' && event.text.unicode <= '9') {
                    if (keyInput.size() < PASS_LEN)
                        keyInput.push_back((char)event.text.unicode);
                }
                if (event.text.unicode == 8 && !keyInput.empty())
                    keyInput.pop_back();
            }

            // ===== JOIN =====
            if (selected != -1 && joinBtn.clicked(event, window))
            {
                int er;
                if (rooms[selected].isPrivate)
                    er = gameData.JoinRoom(selected, keyInput);
                else
                    er = gameData.JoinRoom(selected);

                switch (er)
                {
                    case ROOM_FULL:
                        errorMsg = "Room is full.";
                        break;

                    case ROOM_LOCKED:
                        errorMsg = "Room is locked.";
                        break;

                    case ROOM_PRIVATE:
                        errorMsg = "Room is private.";
                        break;

                    case WRONG_PIN:
                        errorMsg = "Wrong password!";
                        keyInput.clear();
                        wrongKeyCount++;
                        if (wrongKeyCount >= 3) {
                            reason = EndReason::WrongKeyTooMany;
                            state = State::EndConn;
                        }
                        break;

                    case ROOM_PLAYING:
                        errorMsg = "Room is already playing.";
                        break;

                    default: // Success
                        currentRoomIndex = selected;

                        if (gameData.myRoom.n_players == 1)
                            state = State::HostSetting;
                        else
                            state = State::InRoom;

                        return;
                }
            }
        }

        // ============ 60s timeout ============
        float remain = 60.f - timer.getElapsedTime().asSeconds();
        if (remain <= 0.f) {
            reason = EndReason::Timeout;
            state = State::EndConn;
            return;
        }

        // ============================================================ Rendering

        exitBtn.update(window);
        for (auto &b : roomBtn) b.update(window);
        joinBtn.update(window);

        window.setView(uiView);
        window.clear();

        drawBackground(window);

        title.draw(window);
        exitBtn.draw(window);

        // Timer
        sf::Text timerText("Time left: " + std::to_string((int)remain) + "s",
                           font, 26);
        timerText.setFillColor(sf::Color::White);
        timerText.setOutlineColor(sf::Color::Black);
        timerText.setOutlineThickness(2);
        timerText.setPosition(300, 520);
        window.draw(timerText);

        // -------- Room List --------
        for (int i = 0; i < 3; i++)
        {
            Room&   r = rooms[i];
            Button& b = roomBtn[i];

            bool full = r.isFull();

            sf::Color base(210,230,250);
            if (full)      base = sf::Color(180,180,180);
            if (r.inGame)  base = sf::Color(255,215,180);
            if (selected == i) base = sf::Color(255,240,140);

            b.shape.setFillColor(base);
            b.text.setString("");
            b.draw(window);

            // Host
            sf::Text hostTx(
                r.hostName().empty() ? "Host: -" : "Host: " + r.hostName(),
                font, 20
            );
            hostTx.setFillColor(sf::Color::Black);
            hostTx.setPosition(b.shape.getPosition().x + 12,
                               b.shape.getPosition().y + 8);
            window.draw(hostTx);

            // Room name
            sf::Text roomName(r.name, font, 28);
            roomName.setFillColor(sf::Color::Black);
            roomName.setPosition(b.shape.getPosition().x + 130,
                                 b.shape.getPosition().y + 25);
            window.draw(roomName);

            // Status
            std::string status;
            if (r.playerNames.empty())
                status = "Empty room";
            else {
                status = std::to_string(r.n_players) + " players";
                if (r.locked)
                    status += " (Locked)";
            }

            sf::Text st(status, font, 20);
            st.setFillColor(sf::Color::Black);
            st.setPosition(b.shape.getPosition().x + 12,
                           b.shape.getPosition().y + 55);
            window.draw(st);

            // PRIVATE
            if (r.isPrivate) {
                sf::RectangleShape tag({80,22});
                tag.setFillColor(sf::Color(255,200,200));
                tag.setOutlineColor(sf::Color(180,30,30));
                tag.setOutlineThickness(2);
                tag.setPosition(
                    b.shape.getPosition().x + b.shape.getSize().x - 90,
                    b.shape.getPosition().y + 28
                );
                window.draw(tag);

                sf::Text tx("PRIVATE", font, 14);
                tx.setFillColor(sf::Color(180,30,30));
                tx.setPosition(tag.getPosition().x + 8,
                               tag.getPosition().y + 2);
                window.draw(tx);
            }
        }

        // -------- Right Panel --------
        if (selected != -1)
        {
            selectedLabel.draw(window);
            joinBtn.draw(window);

            if (needsKey)
            {
                passLabel.draw(window);

                for (int i = 0; i < PASS_LEN; i++)
                {
                    passBox[i].setPosition(520 + i * 55.f, 240);
                    window.draw(passBox[i]);

                    if (i < (int)keyInput.size()) {
                        sf::Text digit(std::string(1, keyInput[i]), font, 32);
                        digit.setFillColor(sf::Color::Black);
                        digit.setPosition(528 + i * 55.f, 238);
                        window.draw(digit);
                    }
                }
            }

            if (!errorMsg.empty()) {
                sf::Text e(errorMsg, font, 20);
                e.setFillColor(sf::Color::Red);
                e.setPosition(520, 305);
                window.draw(e);
            }
        }

        window.display();
    }
}
