#pragma once
#include "../../net/protocol.h"
#include "../../logic/private_room/private_room_logic.h"

class PrivateRoomHandler {
public:
    explicit PrivateRoomHandler(PrivateRoomLogic& logic);

    Message handleCreatePrivateRoom(const Message& incomingMsg);

private:
    PrivateRoomLogic& privateRoomLogic_;
};
