#include "text_input.h"

TextInput::TextInput() : focused_(false), passwordMode_(false) {
    box_.setOutlineThickness(2.f);
}

void TextInput::setFont(const sf::Font& font) {
    text_.setFont(font);
    placeholder_.setFont(font);
}

void TextInput::setPlaceholder(const std::string& text) {
    placeholder_.setString(text);
    placeholder_.setCharacterSize(20);
    placeholder_.setFillColor(sf::Color(180, 180, 180));
}

void TextInput::setPosition(const sf::Vector2f& pos) {
    box_.setPosition(pos);
    text_.setPosition(pos.x + 10.f, pos.y + 8.f);
    placeholder_.setPosition(pos.x + 10.f, pos.y + 8.f);
}

void TextInput::setSize(const sf::Vector2f& size) {
    box_.setSize(size);
}

void TextInput::setColors(const sf::Color& fill, const sf::Color& outline, const sf::Color& textColor) {
    box_.setFillColor(fill);
    box_.setOutlineColor(outline);
    text_.setFillColor(textColor);
}

void TextInput::setPasswordMode(bool enabled) {
    passwordMode_ = enabled;
}

void TextInput::handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
    if (event.type == sf::Event::MouseButtonPressed) {
        focused_ = box_.getGlobalBounds().contains(mousePos);
        return;
    }

    if (!focused_) return;

    if (event.type == sf::Event::TextEntered) {
        if (event.text.unicode == 8) { // backspace
            if (!content_.empty()) {
                content_.pop_back();
            }
        } else if (event.text.unicode >= 32 && event.text.unicode < 127) {
            content_.push_back(static_cast<char>(event.text.unicode));
        }

        if (passwordMode_) {
            std::string masked(content_.size(), '*');
            text_.setString(masked);
        } else {
            text_.setString(content_);
        }
    }
}

void TextInput::draw(sf::RenderWindow& window) const {
    window.draw(box_);
    if (content_.empty()) {
        window.draw(placeholder_);
    } else {
        window.draw(text_);
    }
}

std::string TextInput::value() const {
    return content_;
}

void TextInput::clear() {
    content_.clear();
    text_.setString("");
}
