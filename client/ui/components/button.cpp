#include "button.h"

Button::Button() {
    shape_.setOutlineThickness(1.5f);
    shape_.setFillColor(idleFillColor_);
    shape_.setOutlineColor(idleOutlineColor_);
}

void Button::setFont(const sf::Font& font) {
    text_.setFont(font);
}

void Button::setEnabled(bool enabled) {
    enabled_ = enabled;
    if (enabled_) {
        // Khôi phục màu gốc (Sáng sủa)
        shape_.setFillColor(idleFillColor_);
        shape_.setOutlineColor(sf::Color(255, 215, 0)); // Viền vàng khi hoạt động
        text_.setFillColor(idleTextColor_);
    } else {
        // Màu khi bị khóa (Tối đậm, không trắng)
        shape_.setFillColor(sf::Color(30, 30, 30));
        shape_.setOutlineColor(sf::Color(60, 60, 60));
        text_.setFillColor(sf::Color(90, 90, 90));
    }
}

void Button::setText(const std::string& text, unsigned int size) {
    text_.setString(sf::String::fromUtf8(text.begin(), text.end()));
    text_.setCharacterSize(size);
    updateTextPosition();
}

void Button::setSize(const sf::Vector2f& size) {
    shape_.setSize(size);
    updateTextPosition();
}

void Button::setPosition(const sf::Vector2f& pos) {
    shape_.setPosition(pos);
    updateTextPosition();
}

void Button::updateTextPosition() {
    // Thuật toán căn giữa chữ hoàn hảo vào giữa Button
    sf::FloatRect textRect = text_.getLocalBounds();
    text_.setOrigin(textRect.left + textRect.width / 2.0f, 
                    textRect.top + textRect.height / 2.0f);
    
    sf::Vector2f shapeCenter(
        shape_.getPosition().x + shape_.getSize().x / 2.0f,
        shape_.getPosition().y + shape_.getSize().y / 2.0f
    );
    text_.setPosition(shapeCenter);
}

void Button::setColors(const sf::Color& fill, const sf::Color& outline, const sf::Color& textColor) {
    idleFillColor_ = fill;
    idleOutlineColor_ = outline;
    idleTextColor_ = textColor;
    
    if (enabled_) {
        shape_.setFillColor(fill);
        shape_.setOutlineColor(outline);
        text_.setFillColor(textColor);
    }
}

void Button::setCallback(std::function<void()> cb) {
    callback_ = std::move(cb);
}

void Button::handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
    if (!enabled_) return; // Khóa rồi thì không nhận event

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        if (shape_.getGlobalBounds().contains(mousePos)) {
            // Hiệu ứng nhấn nhẹ (đổi màu tạm thời)
            shape_.setOutlineThickness(3.f);
            if (callback_) callback_();
        }
    }
    
    if (event.type == sf::Event::MouseButtonReleased) {
        shape_.setOutlineThickness(1.5f);
    }
}

bool Button::isClicked(const sf::Vector2f& mousePos) const {
    return enabled_ && shape_.getGlobalBounds().contains(mousePos);
}

void Button::draw(sf::RenderWindow& window) const {
    window.draw(shape_);
    window.draw(text_);
}