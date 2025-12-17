#include "app/app.hpp"

std::string servip = "127.0.0.1";

std::vector<Room> rooms;
GamePlay gameData;

int currentRoomIndex = -1;

// UI TEST
bool UI_TEST_MODE = false;


int main(int argc, char* argv[])
{
    if(argc != 2) {
        std::cout << "[Warning] Assuming server IP " << servip << std::endl;
        std::cout << "          To specify IP, type ./main <serverIP>" << std::endl;
    }
    else servip = std::string(argv[1]);
    
    initBackground();

    sf::RenderWindow window(
        sf::VideoMode(800, 600),
        "Cat In The Sack",
        sf::Style::Default
    );
    window.setVerticalSyncEnabled(true);


    bool texturesLoaded = false;
    if (GameCardResources::getInstance().loadTextures("assets/")) {
        std::cout << "[Info] Card textures loaded from 'assets/'" << std::endl;
        texturesLoaded = true;
    } 

    if (!texturesLoaded) {
        std::cerr << "[Warning] Failed to load card textures! Cards will display as red blocks." << std::endl;
        std::cerr << "          Please ensure 'card_0.png' through 'card_9.png' are in 'sack-cli/assets/'." << std::endl;
    }

    State state = State::UsernameInput;
    EndReason reason = EndReason::None;
    std::string username;

    rooms = std::vector<Room>(3);
    for (int i = 0; i < 3; i++)
        rooms[i] = Room(i);


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

            // 起始手牌 
            case State::GameStart:
                runStartHandPage(window, state, reason, username);
                break;
            
            // 棄牌
            case State::Discard:
                runDiscardPage(window, state, reason, username);
                break;

            // 正式遊戲出牌階段
            case State::Game:
                runPlayPhasePage(window, state, reason, username);
                break;

            case State::Settlement:
                runSettlementPage(window, state, username);
                break;
                
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
