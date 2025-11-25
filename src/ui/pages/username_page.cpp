#include "ui/pages/username_page.hpp"
#include <algorithm>
#include <cctype>

// 外部變數
extern sf::View uiView;
extern void drawBackground(sf::RenderWindow&); // background繪製函式
extern GamePlay gameData;
extern const std::string servip;
extern bool UI_TEST_MODE;


// UI 常數
namespace UI {
    constexpr float WIDTH   = 800.f;
    constexpr float HEIGHT  = 600.f;

    // Panel（白色底板）
    constexpr float PANEL_W = 550.f;
    constexpr float PANEL_H = 350.f;

    constexpr float CENTER_X = WIDTH  / 2.f;
    constexpr float CENTER_Y = HEIGHT / 2.f;

    // 各元件與面板中心的距離
    constexpr float TITLE_Y_OFFSET  = -125.f;
    constexpr float PROMPT_Y_OFFSET = -65.f;
    constexpr float INPUT_Y_OFFSET  = 5.f;
    constexpr float ERROR_Y_OFFSET  = 75.f;
    constexpr float BUTTON_Y_OFFSET = 125.f;

    // Username 輸入框
    constexpr float INPUT_W = 350.f;
    constexpr float INPUT_H = 50.f;

    // start
    constexpr float BTN_W   = 200.f;
    constexpr float BTN_H   = 60.f;

    // exit
    constexpr float EXIT_W  = 120.f;
    constexpr float EXIT_H  = 40.f;

    // rule
    constexpr float RULE_W  = 120.f;
    constexpr float RULE_H  = 40.f;
}

// ============================================================================
// 在連線失敗時，將 START 按鈕恢復成一般狀態
// ============================================================================
static void resetStartButton(Button& btn, Label& err, bool& connecting)
{
    // 1. 重設連線狀態
    connecting = false; 

    // 2. 重設按鈕
    btn.setDisabled(false);
    btn.text.setString("START");
    btn.centerText();

    // 3. 顯示錯誤訊息
    err.set("Connection failed. Try again.");
    err.centerText();
}

// ============================================================================
// Username Page
// ============================================================================
void runUsernamePage(
    sf::RenderWindow& window,
    State& state,
    std::string& username,
    EndReason& reason
){
    sf::Font font;
    loadFontSafe(font);

    // -------------------- Panel（居中） --------------------
    sf::RectangleShape panel({UI::PANEL_W, UI::PANEL_H});
    panel.setFillColor(sf::Color(255,255,255,230));
    panel.setOutlineColor(sf::Color(100,100,100));
    panel.setOutlineThickness(4);
    panel.setOrigin(UI::PANEL_W/2.f, UI::PANEL_H/2.f);
    panel.setPosition(UI::CENTER_X, UI::CENTER_Y);

    // -------------------- Title（居中） --------------------
    Label title(
        &font, "Cat In The Sack",
        UI::CENTER_X,
        UI::CENTER_Y + UI::TITLE_Y_OFFSET,
        56, sf::Color::White, sf::Color::Black, 4
    );
    title.centerText();

    // -------------------- Prompt（居中） --------------------
    Label prompt(
        &font, "Enter Username",
        UI::CENTER_X,
        UI::CENTER_Y + UI::PROMPT_Y_OFFSET,
        32, sf::Color::Black, sf::Color::Transparent, 0
    );
    prompt.centerText();

    // -------------------- TextBox（居中 + Placeholder） --------------------
    TextBox usernameBox(
        &font,
        UI::CENTER_X,
        UI::CENTER_Y + UI::INPUT_Y_OFFSET,
        UI::INPUT_W, UI::INPUT_H,
        true 
    );
    usernameBox.setPlaceholder("Nickname (A-Z, 0-9, _)");

    // -------------------- START 按鈕 --------------------
    Button startBtn(
        &font, "START",
        UI::CENTER_X,
        UI::CENTER_Y + UI::BUTTON_Y_OFFSET,
        UI::BTN_W, UI::BTN_H,
        true
    );

    // -------------------- Exit（右上角） --------------------
    Button exitBtn(
        &font, "Exit",
        UI::WIDTH - UI::EXIT_W/2.f - 20.f,
        20.f + UI::EXIT_H/2.f,
        UI::EXIT_W, UI::EXIT_H,
        true
    );

    // -------------------- Rules 按鈕 --------------------
    Button rulesBtn(
        &font, "Rules",
        UI::WIDTH - UI::RULE_W/2.f - 20.f - 130.f,
        20.f + UI::RULE_H/2.f,
        UI::RULE_W, UI::RULE_H,
        true
    );

    // -------------------- Error Label（居中） --------------------
    Label error(
        &font, "",
        UI::CENTER_X,
        UI::CENTER_Y + UI::ERROR_Y_OFFSET,
        22, sf::Color(220,50,50),
        sf::Color::Transparent, 0
    );
    error.centerText();

    bool connecting = false;

    // ========================================================================
    // Main Loop
    // ========================================================================
    while (window.isOpen() && state == State::UsernameInput)
    {
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed)
                window.close();

            // --------- Rules Page ---------
            if (rulesBtn.clicked(e, window)) {
                state = State::Rules;
                return;
            }

            // Exit
            if (exitBtn.clicked(e, window)) {
                window.close();
                return;
            }

            // 如果正在連線，則忽略所有輸入事件
            if (connecting) {
                // 允許按 Enter 取消連線嘗試 (可選)
                if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) {
                    resetStartButton(startBtn, error, connecting);
                }
                continue; 
            }

            // 清除錯誤訊息 (僅在非連線狀態下)
            if ((e.type == sf::Event::TextEntered || e.type == sf::Event::KeyPressed))
            {
                if (!error.text.getString().isEmpty()) {
                    error.set("");
                    error.centerText();
                }
            }

            // 禁止空白
            if (e.type == sf::Event::TextEntered && e.text.unicode == ' ')
                continue;

            usernameBox.handleEvent(e, window);
            
            bool tryConnect = false;

            // 檢查 Enter 鍵觸發
            if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Enter) {
                tryConnect = true;
            }
            
            // 檢查 START 按鈕點擊
            if (startBtn.clicked(e, window)) {
                tryConnect = true;
            }

            if (tryConnect)
            {
                username = usernameBox.buffer;
                bool validationFailed = false;

                // 1. 驗證 username
                if (username.empty()) {
                    error.set("Username cannot be empty.");
                    validationFailed = true;
                }
                else if (username.size() > 14) {
                    error.set("Max 14 characters.");
                    validationFailed = true;
                }
                else {
                    for (char c : username) {
                        if (!(std::isalnum(c) || c == '_')) {
                            error.set("Only A-Z, 0-9, and _ allowed.");
                            validationFailed = true;
                            break;
                        }
                    }
                }
                
                if (validationFailed) {
                    error.centerText();
                    continue; // 跳過連線步驟
                }

                

                // 2. 進入連線狀態
                connecting = true;
                startBtn.setDisabled(true);
                startBtn.text.setString("CONNECTING...");
                startBtn.centerText();

                error.set("Connecting to server...");
                error.centerText();

                // ⭐⭐⭐ UI TEST MODE：不連 server，直接進入房間清單 ⭐⭐⭐
                if (UI_TEST_MODE) {
                    gameData = GamePlay();   // no socket
                    state = State::RoomInfo;
                    return;
                }

                // 3. 嘗試連線
                GamePlay play(servip.c_str(), username);

                // 4. 處理連線結果
                if (!play.isConnected()) {
                    // 連線失敗
                    resetStartButton(startBtn, error, connecting);
                }
                else {
                    // 連線成功
                    gameData = play;
                    state = State::RoomInfo;
                    return;
                }
            } // end tryConnect
        } // end while(pollEvent)

        // Hover (非連線狀態下才更新)
        if (!connecting) {
            startBtn.update(window);
        }
        exitBtn.update(window);
        rulesBtn.update(window);

        // Draw
        window.setView(uiView);
        window.clear();
        drawBackground(window);

        window.draw(panel);
        title.draw(window);
        prompt.draw(window);
        usernameBox.draw(window);
        startBtn.draw(window);
        exitBtn.draw(window);
        rulesBtn.draw(window);
        error.draw(window);

        window.display();
    }
}