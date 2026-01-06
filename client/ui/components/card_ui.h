#pragma once
#include <SFML/Graphics.hpp>
#include <string>

enum Suit { SPADES = 0, CLUBS = 1, DIAMONDS = 2, HEARTS = 3 };

struct CardInfo {
    int id;
    std::string rank;     
    std::wstring symbol;  
    sf::Color color;      
};

class CardUI : public sf::Drawable, public sf::Transformable {
private:
    CardInfo m_info;
    sf::RectangleShape m_shape;
    sf::RectangleShape m_shadow;
    sf::Text m_txtMain;      
    sf::Text m_txtCorner;    
    sf::Text m_txtSuitSmall; 
    
    sf::Font* m_font;
    bool m_isSelected;
    bool m_isHovered;
    sf::Color m_suitColor;

    void decodeID(int id); // Định nghĩa ở .cpp
    void setupUI();        // Định nghĩa ở .cpp
    void setupText(sf::Text& txt, sf::String str, int size, sf::Color color, float x, float y); // Định nghĩa ở .cpp
    void centerText(sf::Text& txt, float offsetX, float offsetY); // Định nghĩa ở .cpp

public:
    explicit CardUI(int id, sf::Font& font); // Constructor

    int getID() const; 
    bool isSelected() const;
    void toggleSelect();
    void setHover(bool state);

    void setSize(const sf::Vector2f& size); // Định nghĩa ở .cpp

    bool contains(float x, float y) const; // Định nghĩa ở .cpp
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override; // Định nghĩa ở .cpp
};