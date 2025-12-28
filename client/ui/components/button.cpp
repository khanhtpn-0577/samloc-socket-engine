#include "button.h"

Button::Button() {
    shape_.setOutlineThickness(2.f);
}

void Button::setFont(const sf::Font& font) {
    text_.setFont(font);
}

void Button::setText(const std::string& text, unsigned int size) {
    text_.setString(text);
    text_.setCharacterSize(size);
}

void Button::setSize(const sf::Vector2f& size) {
    shape_.setSize(size);
}

void Button::setPosition(const sf::Vector2f& pos) {
    shape_.setPosition(pos);
    text_.setPosition(pos.x + 12.f, pos.y + 8.f);
}

void Button::setColors(const sf::Color& fill, const sf::Color& outline, const sf::Color& textColor) {
    shape_.setFillColor(fill);
    shape_.setOutlineColor(outline);
    text_.setFillColor(textColor);
}

void Button::setCallback(std::function<void()> cb) {
    callback_ = std::move(cb);
}

void Button::handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        if (shape_.getGlobalBounds().contains(mousePos)) {
            if (callback_) {
                callback_();
            }
        }
    }
}

bool Button::isClicked(const sf::Vector2f& mousePos) const {
    return shape_.getGlobalBounds().contains(mousePos);
}

void Button::draw(sf::RenderWindow& window) const {
    window.draw(shape_);
    window.draw(text_);
}
