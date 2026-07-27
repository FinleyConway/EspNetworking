#pragma once

#include "common/core/packet_registry.hpp"

#include "common/messages/handshake.hpp"

namespace common {
    using registry_t = packet_registry_impl_t<
        esp_init_request_t,
        esp_init_response_t
    >;
}