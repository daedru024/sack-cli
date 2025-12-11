#pragma once
#include <SFML/Graphics.hpp>

inline const int CARD_VALUES[10] = {
    -8, -5, 0, 3, 5, 8, 11, 15, -9, 9
};
//   ID:  0   1  2  3  4  5   6   7   8   9

enum class CardType {
    Cat,
    BigDog,
    SmallDog,
    Rabbit
};

inline CardType getCardType(int cardId) {
    if (cardId == 2) return CardType::Rabbit;  // 2 → 兔子
    if (cardId == 8) return CardType::BigDog;  // 8 → 大狗 (-9)
    if (cardId == 9) return CardType::SmallDog; // 9 → 小狗 (9)
    return CardType::Cat;                      // 其他 → 貓
}

inline sf::Color cardFillColor(CardType t) {
    // 普通貓: 淺黃色 (255,240,180)
    // 大狗:   紫色 (147,112,219)
    // 小狗:   棕色 (205,133,63)
    // 兔子:   粉紅色 (255,105,180)
    switch (t) {
        case CardType::Cat:     return sf::Color(255,240,180);
        case CardType::BigDog:  return sf::Color(147,112,219);
        case CardType::SmallDog:return sf::Color(205,133,63);
        case CardType::Rabbit:  return sf::Color(255,105,180);
    }
    return sf::Color::White;
}

inline int cardValue(int cardId) {
    if (cardId < 0 || cardId >= 10) return 0;
    return CARD_VALUES[cardId];
}

class GameCardResources {
public:
    static GameCardResources& getInstance() {
        static GameCardResources instance;
        return instance;
    }

    // 載入圖片 (需在 main 中呼叫)
    bool loadTextures(const std::string& basePath) {
        bool success = true;
        for (int i = 0; i < 10; ++i) {
            std::string filename = basePath + "card_" + std::to_string(i) + ".png";
            if (!textures[i].loadFromFile(filename)) {
                std::cerr << "Failed to load: " << filename << std::endl;
                // 載入失敗建立預設紅色圖
                textures[i].create(100, 140);
                sf::Image img;
                img.create(100, 140, sf::Color::Red);
                textures[i].update(img);
                success = false;
            } else {
                textures[i].setSmooth(true);
            }
        }
        return success;
    }

    const sf::Texture& getTexture(int index) const {
        if (index >= 0 && index < 10) {
            return textures[index];
        }
        static sf::Texture empty;
        return empty;
    }

private:
    GameCardResources() {}
    sf::Texture textures[10]; 
};

