#pragma once

#include <memory>

#include <asio.hpp>

#include "host/networking/tcp/tcp_callbacks.hpp"
#include "host/networking/tcp/tcp_io_state.hpp"

#include "common/api/registry.hpp"
#include "common/api/types.hpp"

namespace host {
    namespace ip = asio::ip;

    /// @brief A tcp esp client that can send and receive messages from
    class tcp_connection_t : public std::enable_shared_from_this<tcp_connection_t> {
    public:
        using pointer_t = std::shared_ptr<tcp_connection_t>;

        /// @brief Create a new tcp esp client
        /// @param io_context The server io context
        /// @return A pointer to a newly created client
        static pointer_t create(asio::io_context& io_context);

        /// @brief The client socket
        /// @return The tcp socket
        ip::tcp::socket& get_socket();

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

        /// @brief Allow to stop or start receiving messages from this client
        /// @param enable The toggle state
        void set_receiving_state(bool enable);

        /// @brief Disconnect the client
        /// @return Has the client been disconnected properly
        bool disconnect();

    private:
        explicit tcp_connection_t(asio::io_context& io_context);

    private:
        ip::tcp::socket m_socket;
        tcp_io_state_t m_io_state;
    };

    using tcp_connection_ptr_t = tcp_connection_t::pointer_t;
}