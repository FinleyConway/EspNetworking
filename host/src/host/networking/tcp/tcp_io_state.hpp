#pragma once

#include <deque>
#include <cstdint>
#include <functional>
#include <system_error>

#include <asio.hpp>

#include "host/networking/tcp/tcp_callbacks.hpp"

#include "common/api/registry.hpp"
#include "common/api/types.hpp"

namespace host {
    namespace ip = asio::ip;

    /// @brief Handles to sending and receiving to/from the client
    class tcp_io_state_t {
    public:
        /// @brief Construct tcp_io_state_t
        /// @param socket The client socket
        tcp_io_state_t(ip::tcp::socket& socket);

        /// @brief Initialise the client with context properties
        /// @param id The client id
        /// @param registry The tcp messaging system
        /// @param callback The callback for when this client disconnects
        void set_spec(common::esp_id_t id, common::registry_t& registry, on_disconnect_fn&& callback);

        /// @brief Allow to send a message to the client
        /// @param payload The message byte stream
        /// @param bytes The size of the message byte stream
        /// @return Has the message been sent
        bool send(common::registry_t::payload_t&& payload, size_t bytes);

        /// @brief Start receiving messages from client
        void start_receiving();

        /// @brief Stop receiving messages from client
        void stop_receiving();

        /// @brief Stop all form of sending and receiving
        void stop_io();

    private:
        enum class io_state_t {
            idle,
            reading_payload,
            writing_payload
        };

    private:
        void write();

        void read_id();

        void read_payload(const std::error_code& id_ec, size_t);

        bool has_or_handle_io_error(const std::error_code& ec);

    private:
        struct write_state_t {
            const common::registry_t::payload_t payload;
            const size_t bytes = 0;
        };

        struct read_state_t {
            common::registry_t::packet_id_t id = 0;
            common::registry_t::payload_t payload;
        };

    private:
        common::esp_id_t m_id = 0;
        ip::tcp::socket& m_socket;
        common::registry_t* m_registry = nullptr;

        bool m_can_receive = false;
        read_state_t m_read_state;
        std::deque<write_state_t> m_write_state_queue;
        io_state_t m_io_read_state = io_state_t::idle;
        io_state_t m_io_write_state = io_state_t::idle;

        on_disconnect_fn m_disconnect_callback;
    };
}