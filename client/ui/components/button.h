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
    void setEnabled(bool enabled);

    // Hàm lấy vị trí cần thiết cho InGameState
    sf::Vector2f getPosition() const { return shape_.getPosition(); }
    sf::Vector2f getSize() const { return shape_.getSize(); }

    void handleEvent(const sf::Event& event, const sf::Vector2f& mousePos);
    bool isClicked(const sf::Vector2f& mousePos) const;
    void draw(sf::RenderWindow& window) const;

private:
    void updateTextPosition(); // Hàm hỗ trợ căn giữa chữ

    sf::RectangleShape shape_;
    sf::Text text_;
    std::function<void()> callback_;
    bool enabled_ = true;

    // Lưu lại màu gốc để khôi phục khi enabled lại
    sf::Color idleFillColor_ = sf::Color(60, 60, 60);
    sf::Color idleOutlineColor_ = sf::Color(255, 255, 255, 100);
    sf::Color idleTextColor_ = sf::Color::White;
};