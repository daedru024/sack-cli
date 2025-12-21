#include "app/app.hpp"

std::string servip = "127.0.0.1";

std::vector<Room> rooms;
GamePlay gameData;

int currentRoomIndex = -1;

// UI TEST
bool UI_TEST_MODE = false;


int main(int argc, char* argv[])
{
    if(argc == 2) {
        // 如果有指令參數 (./main 172.x.x.x)，優先使用
        servip = std::string(argv[1]);
        std::cout << "[Info] Using IP from command line: " << servip << std::endl;
    }
    else {
        // 如果沒有參數，嘗試讀取 config.txt
        std::ifstream configFile("config.txt");
        if (configFile.is_open()) {
            std::string line;
            if (std::getline(configFile, line) && !line.empty()) {
                // 去除前後可能的空白或換行符號
                line.erase(0, line.find_first_not_of(" \t\n\r"));
                line.erase(line.find_last_not_of(" \t\n\r") + 1);
                
                servip = line;
                std::cout << "[Info] Read server IP from config.txt: " << servip << std::endl;
            }
            configFile.close();
        }
        else {
            std::cout << "[Warning] config.txt not found. Using default IP: " << servip << std::endl;
        }
    }
    
    //gameData.servip = servip; 

    
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
