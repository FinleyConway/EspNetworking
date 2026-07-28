#pragma once

#include <mutex>
#include <thread>
#include <utility>
#include <exception>
#include <system_error>
#include <unordered_map>

#include <asio.hpp>

#include "host/networking/tcp/tcp_callbacks.hpp"
#include "host/networking/tcp/tcp_connection.hpp"

#include "host/protocol/mdns_service.hpp"

#include "common/api/types.hpp"
#include "common/api/registry.hpp"

namespace host {
    namespace ip = asio::ip;

    /// @enum tcp_status_t
    /// @brief The return context of the called function
    enum class tcp_status_t {
        success,                ///< Operation succeeded
        already_accepting,      ///< Already in requested accepting state
        fail_to_accept,         ///< Failed to bind/listen socket
        unknown_client,         ///< Unknown client id
        no_client_connection    ///< Client no longer connected
    };

    /// @brief The tcp server that handles the i/o of messages between the host and clients
    class tcp_server_t {
    public:
        /// @brief Construct tcp_server_t
        tcp_server_t();

        /// @brief Destroy tcp_server_t
        ~tcp_server_t();

        /// @brief Get the server io context
        /// @return The server handle
        asio::io_context& get_io_context();
        
        /// @brief Start the server and start broadcasting all registered mDNS services
        void start();

        /// @brief Check if the server is running
        /// @return Is the server running
        bool is_running() const;

        void shutdown();

        /// @brief Start or stop accepting messages from all connected clients
        /// @param enable Stop or start accepting
        /// @retval tcp_status_t::success: Operation succeeded
        /// @retval tcp_status_t::already_accepting: Already in requested state
        /// @retval tcp_status_t::fail_to_bind: Failed to bind or listen
        tcp_status_t toggle_accepting(bool enable);

        /// @brief Send a message to a client
        /// @tparam T A registered message type 
        /// @param client_id The client id
        /// @param data A registered message type data
        /// @retval tcp_status_t::success: Operation succeeded
        /// @retval tcp_status_t::unknown_client: Unknown client id
        /// @retval tcp_status_t::no_client_connection: Client no longer connected
        template<typename T>
        tcp_status_t send_to_client(common::esp_id_t client_id, const T& data) {
            std::lock_guard lock(m_connection_mutex);

            auto it = m_connections.find(client_id);
            if (it == m_connections.end()) {
                return tcp_status_t::unknown_client;
            }

            const bool success = it->second->send(
                m_registry.create(data),
                m_registry.packet_size<T>()
            );

            if (!success) return tcp_status_t::no_client_connection;

            return tcp_status_t::success;
        }

        /// @brief Allow to stop or start receiving messages from this client
        /// @param enable The toggle state
        void receive_from_client(bool enable);

        /// @brief Disconnect client
        /// @param client_id The client id
        /// @retval tcp_status_t::success: Operation succeeded
        /// @retval tcp_status_t::unknown_client: Unknown client id
        /// @retval tcp_status_t::no_client_connection: Client no longer connected
        tcp_status_t disconnect_client(common::esp_id_t client_id);

        /// @brief Provide the callback for registered T
        /// @tparam T A registered message type 
        /// @tparam fn The callback when receiving a message, void(void* ctx, const T&)
        /// @param ctx A pointer ctx that can be called with the callback
        template<typename T, auto Fn>
        void register_receive_callback(void* ctx) {
            m_registry.template register_callback<T, Fn>(ctx);
        }

        /// @brief Provide the callback for when a client connects
        /// @param callback The connect callback, void(common::esp_id_t)
        void register_on_connect(on_connect_fn&& callback);

        /// @brief Provide the callback for when a client disconnects
        /// @param callback The disconnect callback, void(common::esp_id_t)
        void register_on_disconnect(on_disconnect_fn&& callback);

    private:
        void on_connect(common::esp_id_t id);

        void on_disconnect(common::esp_id_t id);
        
        void wait_for_connection();

        void create_new_connection(tcp_connection_ptr_t new_connection, const std::error_code& ec);

    private:
        asio::io_context m_io_context;
        ip::tcp::acceptor m_acceptor;
        ip::tcp::endpoint m_endpoint;
        host::mdns_service_t m_mdns_service;

        std::thread m_io_thread;
        std::mutex m_connection_mutex;
        asio::executor_work_guard<asio::io_context::executor_type> m_work_guard;

        std::unordered_map<common::esp_id_t, tcp_connection_ptr_t> m_connections;
        common::esp_id_t m_connection_count = 0;
        bool m_receiving = true;

        on_connect_fn m_on_connect_callback;
        on_disconnect_fn m_on_disconnect_callback;

        common::registry_t m_registry;
    };
}