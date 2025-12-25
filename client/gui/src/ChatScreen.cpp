#include "../include/ChatScreen.h"
#include <SFML/Window.hpp>
#include <iostream>

ChatScreen::ChatScreen(){
    if(!font.loadFromFile("../gui/assets/DejaVuSans.ttf")){
        std::cerr << "Error loading font\n";
    }

    //Setup title
    titleText.setFont(font);
    titleText.setString("Chat Room");
    titleText.setCharacterSize(30);
    titleText.setPosition(10, 10);
    titleText.setFillColor(sf::Color::White);

    //Setup message input box
    messageInputBox.setSize(sf::Vector2f(400, 30));
    messageInputBox.setPosition(10, 500);
    messageInputBox.setFillColor(sf::Color(200, 200, 200));

    //Setup send button
    sendButton.setSize(sf::Vector2f(80, 30));
    sendButton.setPosition(420, 500);
    sendButton.setFillColor(sf::Color::Green);

    sendButtonText.setFont(font);
    sendButtonText.setString("Send");
    sendButtonText.setCharacterSize(20);
    sendButtonText.setPosition(430, 505);
    sendButtonText.setFillColor(sf::Color::White);

    messageInputText.setFont(font);
    messageInputText.setCharacterSize(20);
    messageInputText.setFillColor(sf::Color::Black);
    messageInputText.setPosition(15, 505);
}

void ChatScreen::run(sf::RenderWindow& window){
    while (window.isOpen()){
        handleEvents(window);
        update();
        render(window);
    }
}

void ChatScreen::handleEvents(sf::RenderWindow& window){
    sf::Event event;
    while (window.pollEvent(event)){
        if(event.type == sf::Event::Closed){
            window.close();
        }
        if(event.type == sf::Event::TextEntered){
            if (event.text.unicode == 13){
                //send message when enter is pressed
                addMessage(userInput);
                userInput.clear();
            } else if (event.text.unicode == 8 && !userInput.empty()){
                //handle backspace
                userInput.erase(userInput.size() - 1, 1);
            }else{
                userInput += event.text.unicode;
            }
        }
    }
}

void ChatScreen::update(){
    messageInputText.setString(userInput);
}

void ChatScreen::render(sf::RenderWindow& window){
    window.clear();

    // Draw title
    window.draw(titleText);

    // Draw the message input box and button
    window.draw(messageInputBox);
    window.draw(sendButton);
    window.draw(sendButtonText);

    // Draw all messages
    int offsetY = 50;
    for (auto& msg : messageList) {
        msg.setPosition(10, offsetY);
        window.draw(msg);
        offsetY += 40; // Add space between messages
    }

    // Draw the current message being typed
    window.draw(messageInputText);
    
    window.display();
}

void ChatScreen::addMessage(const std::string& message){
    //if message list exceeds the max limit, remove the first message
    if(messageList.size() >= MAX_MESSAGES){
        messageList.erase(messageList.begin());
    }

    //add new message to the list
    sf::Text newMessage(message, font, 20);
    newMessage.setFillColor(sf::Color::White);
    messageList.push_back(newMessage);
}


