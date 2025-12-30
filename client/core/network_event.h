#pragma once

#include <string>
#include <variant>
#include <cstdint>
#include "../net/protocol.h"

struct RawMessageEvent {
    Message message;
};

struct DisconnectEvent {
    std::string reason;
};

using NetworkEventPayload = std::variant<RawMessageEvent, DisconnectEvent>;

struct NetworkEvent {
    NetworkEventPayload payload;
};
