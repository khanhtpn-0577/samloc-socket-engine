#pragma once

#include "../game_state.h"
#include "../state_context.h"
#include "../components/button.h"
#include "../components/text_input.h"
#include <vector>
#include <string>

/**
 * PrivateChatState
 *  - Hiển thị danh sách bạn bè (dummy)
 *  - Chọn bạn → mở cửa sổ chat bên phải
 */

class PrivateChatState: public GameState{
    public:
        explicit PrivateChatState(StateContext& ctx); //constructor nhan vao state context

        void onEnter() override; //ham duoc goi khi vao state
        void onExit() override; //ham duoc goi khi thoat state
        
        void handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) override;
        void update(float dt) override;
        void draw(sf::RenderWindow& window) override;

    private:
        struct FriendItem {
            uint32_t userId;
            std::string username;
            sf::Text nameText;
            Button selectButton;
        };
        
        StateContext& ctx_;

        // ===== Layout =====
        sf::RectangleShape leftPanel_;
        sf::RectangleShape rightPanel_;
        sf::Text titleText_;

        // ===== Friend list =====
        std::vector<FriendItem> friends_;
        uint32_t selectedFriendId_;

        // ===== Chat UI =====
        sf::Text chatTitle_;
        TextInput messageInput_;
        Button sendButton_;
    
    private:
        void buildDummyFriendList();
        void rebuildFriendListLayout();
        void requestFriendList();
        void buildFriendListFromData(const std::vector<FriendInfo>& friends);

};