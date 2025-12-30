#pragma once

#include <SFML/Graphics.hpp>
#include <string>

class TextInput {
public:
    TextInput();

    void setFont(const sf::Font& font);
    void setPlaceholder(const std::string& text);
    void setPosition(const sf::Vector2f& pos);
    void setSize(const sf::Vector2f& size);
    void setColors(const sf::Color& fill, const sf::Color& outline, const sf::Color& textColor);
    void setPasswordMode(bool enabled);

    void handleEvent(const sf::Event& event, const sf::Vector2f& mousePos);
    void draw(sf::RenderWindow& window) const;

    std::string value() const;
    void clear();

private:
    bool focused_;
    bool passwordMode_;
    sf::RectangleShape box_;
    sf::Text text_;
    sf::Text placeholder_;
    std::string content_;
};
