#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <functional>

class Button {
public:
    Button();

    void setFont(const sf::Font& font);
    void setText(const std::string& text, unsigned int size = 20);
    void setSize(const sf::Vector2f& size);
    void setPosition(const sf::Vector2f& pos);
    void setColors(const sf::Color& fill, const sf::Color& outline, const sf::Color& textColor);
    void setCallback(std::function<void()> cb);

    void handleEvent(const sf::Event& event, const sf::Vector2f& mousePos);
    bool isClicked(const sf::Vector2f& mousePos) const;
    void draw(sf::RenderWindow& window) const;

private:
    sf::RectangleShape shape_;
    sf::Text text_;
    std::function<void()> callback_;
};
