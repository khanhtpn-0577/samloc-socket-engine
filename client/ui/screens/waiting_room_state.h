#pragma once

#include "../game_state.h"
#include "../state_context.h"
#include "../components/button.h"
#include "../../handlers/room/room_structs.h" 
#include <unordered_map>
#include <vector>
#include <string>
#include <utility>

class WaitingRoomState : public GameState {
public:
    explicit WaitingRoomState(StateContext& ctx);
    
    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    void updateMembers(const std::vector<RoomMember>& newMembers);
    void submitChat();


private:
    StateContext& ctx_;
    
    sf::RectangleShape background_;
    sf::RectangleShape headerBar_;
    sf::Text roomNameText_;
    sf::Text subTitleText_; 

    Button btnLeave_;
    Button btnReady_;
    Button btnChat_;
    Button btnInviteFriend_;


    bool isMyReady_;
    std::vector<RoomMember> members_;
    
    RoomInfo currentRoomInfo_;

    struct PlayerSlot {
        sf::RectangleShape panelBg;
        sf::CircleShape avatarBg;   
        sf::Text avatarLetter;      
        sf::Text nameText;
        sf::Text balanceText;
        sf::Text readyStatusText;   
        
        sf::Vector2f position;
        int playerId = -1;

        void setup(sf::Font& font);
        void setContent(const RoomMember& member);
        void setEmpty();
        void draw(sf::RenderWindow& window);
        
    };

    PlayerSlot slots_[4];
    std::unordered_map<int, int> playerIdToSlot_;
    
    void setupSlotsLayout();
    void refreshSlotDisplay();
    void onReadyClicked();
    void onLeaveClicked();


    // UI state cho popup invite
    bool isInvitePopupVisible_ = false;

    // friend list cache
    std::vector<std::pair<uint32_t, std::pair<std::string, double>>> friendList_;

    sf::RectangleShape invitePopupBg_;
    sf::Text invitePopupTitle_;
    sf::Text invitePopupListText_;
    Button btnInvitePopupClose_;


    struct InviteFriendRow {
        uint32_t userId;
        std::string username;
        long long balance;
        bool online;

        sf::RectangleShape bg;
        sf::Text nameText;
        sf::Text balanceText;
        sf::Text statusText;
        Button inviteBtn;
    };

    std::vector<InviteFriendRow> inviteFriendRows_;

    // ===== Challenge result popup =====
    bool isChallengePopupVisible_ = false;
    bool challengeResultSuccess_ = false;
    sf::RectangleShape challengePopupBg_;
    sf::Text challengePopupText_;
    Button challengePopupOkBtn_;



    // ==== Chat popup ====
    bool chatPopupVisible_ = false;
    sf::RectangleShape chatPopupBg_;
    sf::Text chatTitleText_;
    sf::Text chatInputText_;
    sf::RectangleShape chatInputBox_;
    std::string chatInputBuffer_;
    Button btnChatSend_;
    Button btnChatClose_;

    struct ChatBubble {
        uint32_t senderId;
        std::string text;
        float ttl;
    };

    std::vector<ChatBubble> chatBubbles_;


};