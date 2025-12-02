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
    // 方案 A:
    // 普通貓: 淺黃色 (255,240,180)
    // 大狗:   深紅色 (200,50,50)
    // 小狗:   橘色   (255,150,0)
    // 兔子:   淺綠色 (150,255,150)
    switch (t) {
        case CardType::Cat:     return sf::Color(255,240,180);
        case CardType::BigDog:  return sf::Color(200,50,50);
        case CardType::SmallDog:return sf::Color(255,150,0);
        case CardType::Rabbit:  return sf::Color(150,255,150);
    }
    return sf::Color::White;
}

inline int cardValue(int cardId) {
    if (cardId < 0 || cardId >= 10) return 0;
    return CARD_VALUES[cardId];
}
