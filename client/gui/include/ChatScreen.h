#ifndef CHATSCREEN_H
#define CHATSCREEN_H

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <string>

class ChatScreen {
public:
    ChatScreen();
    void run(sf::RenderWindow& window);

private:
    void handleEvents(sf::RenderWindow& window);
    void update();
    void render(sf::RenderWindow& window);
    void addMessage(const std::string& message);

    //GUI components
    sf::Font font;
    sf::Text messageInputText;
    sf::Text sendButtonText;
    sf::RectangleShape sendButton;
    sf::RectangleShape messageInputBox;
    sf::Text titleText;

    std::vector<sf::Text> messageList;
    
    std::string userInput;

    static const int MAX_MESSAGES = 50;
};

#endif