#pragma once

#include "../game_state.h"
#include "../state_context.h"
#include "../components/button.h"
#include "../../handlers/room/room_structs.h"
#include <vector>
#include <string>

class RoomListState : public GameState {
public:
    explicit RoomListState(StateContext& ctx);
    
    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    StateContext& ctx_;
    sf::RectangleShape background_;
    sf::Text titleText_;
    Button btnBack_;
    Button btnLogout_;

    struct RoomUIItem {
        sf::RectangleShape shape;
        sf::Text nameText;
        sf::Text infoText;
        int roomId;
        sf::Color baseColor;
    };
    std::vector<RoomUIItem> uiRooms_;
    std::vector<RoomInfo> roomData_;
    int lastClickedRoomId_ = 0;

    bool isPopupVisible_;
    sf::RectangleShape popupOverlay_;
    sf::RectangleShape popupBg_;
    sf::Text popupText_;
    Button btnPopupOk_;

    // ===== Create Private Room =====
    Button btnCreateRoom_;

    // Popup create room
    bool isCreateRoomPopupVisible_ = false;

    sf::RectangleShape overlay_;

    sf::RectangleShape createPopupBg_;
    sf::RectangleShape createPopupHeader_;
    sf::Text createPopupTitle_;

    sf::Text lbRoomName_;
    sf::Text lbRoomType_;
    sf::Text lbBet_;

    sf::RectangleShape inputRoomNameBox_;
    sf::RectangleShape inputBetBox_;
    sf::Text inputRoomNameText_;
    sf::Text inputBetText_;
    sf::Text hintRoomName_;
    sf::Text hintBet_;

    Button btnTypeDatCuoc_;
    Button btnTypeDemLa_;
    Button btnCreateConfirm_;
    Button btnCreateCancel_;

    // Input state
    enum class CreateField { None, RoomName, Bet };
    CreateField activeField_ = CreateField::None;

    std::string inputRoomName_;
    std::string inputBet_;
    bool isDatCuoc_ = true;

    // helpers
    void openCreateRoomPopup();
    void closeCreateRoomPopup();
    void handleCreatePopupEvent(const sf::Event& event, const sf::Vector2f& mousePos);
    void refreshCreatePopupTexts();
    bool isDigitsOnly(const std::string& s) const;
    void submitCreateRoom();



    void onRoomsLoaded(const std::vector<RoomInfo>& rooms);
    void onJoinResult(bool success, const std::string& message, int roomId, const RoomInfo& roomInfo);
    void showPopup(const std::string& msg);
    void hidePopup();
};