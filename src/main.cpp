#include "app/app.hpp"

const std::string servip = "127.0.0.1";

std::vector<Room> rooms;
GamePlay gameData;

int currentRoomIndex = -1;

// ==========================
// UI Test Mode（不連 Server）
// ==========================
bool UI_TEST_MODE = false;   // ★★★ 重要：改成 false 就恢復正常連線
// ==========================


int main()
{
    initBackground();

    sf::RenderWindow window(
        sf::VideoMode(800, 600),
        "Cat In The Sack",
        sf::Style::Default
    );
    window.setVerticalSyncEnabled(true);

    State state = State::UsernameInput;
    EndReason reason = EndReason::None;
    std::string username;

    rooms = std::vector<Room>(3);
    for (int i = 0; i < 3; i++)
        rooms[i] = Room(i);

    // // ★ UI 測試模式下，預先塞一點假資料讓介面更好看
    // if (UI_TEST_MODE)
    // {
    //     rooms[0].n_players = 3;
    //     rooms[0].playerNames = { "HostA", "Tom", "Jerry", "", "" };
    //     rooms[0].colors = { 0, 2, 4, -1, -1 };
    //     rooms[0].isPrivate = false;

    //     rooms[1].n_players = 1;
    //     rooms[1].playerNames = { "SoloHost", "", "", "", "" };
    //     rooms[1].colors = { -1, -1, -1, -1, -1 };
    //     rooms[1].isPrivate = true;

    //     rooms[2].n_players = 5;
    //     rooms[2].playerNames = { "A", "B", "C", "D", "E" };
    //     rooms[2].colors = { 0, 1, 2, 3, 4 };
    //     rooms[2].isPrivate = false;
    //     rooms[2].locked = true;
    // }


    while (window.isOpen())
    {
        switch (state)
        {
            case State::UsernameInput:
                runUsernamePage(window, state, username, reason);
                break;

            case State::Rules:
                runRulesPage(window, state);
                break;

            case State::RoomInfo:
                runRoomInfoPage(window, state, reason, username);
                break;

            case State::HostSetting:
                if (currentRoomIndex >= 0 &&
                    currentRoomIndex < (int)rooms.size())
                {
                    runHostSettingPage(
                        window, state, reason,
                        rooms[currentRoomIndex],
                        username
                    );
                }
                else {
                    state = State::RoomInfo;
                }
                break;

            case State::InRoom:
                runInRoomPage(window, state, username, reason);
                break;

            case State::GameStart:
            {
                window.setView(uiView);
                window.clear();
                window.draw(g_bgSprite());
                window.draw(g_bgOverlay());

                sf::Font font;
                font.loadFromFile("fonts/SourceHanSansTC-Regular.otf");


                sf::Text tx("Game starting ... (placeholder)", font, 28);
                tx.setFillColor(sf::Color::White);
                tx.setOutlineColor(sf::Color::Black);
                tx.setOutlineThickness(2);
                tx.setPosition(120, 260);
                window.draw(tx);

                window.display();

                // 原本 main.cpp：遊戲先跳回 RoomInfo
                state = State::RoomInfo;
                break;
            }

            case State::ReEstablish:
                state = State::RoomInfo;
                break;

            case State::EndConn:
                runEndConnPage(window, state, reason);
                break;
        }
    }

    return 0;
}
