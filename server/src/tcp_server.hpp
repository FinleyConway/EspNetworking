#pragma once

#include <unordered_map>
#include <iostream> // prolly want better logging
#include <cstdint>
#include <functional>

#include <asio.hpp>

#include "tcp_connection.hpp"

using asio::ip::tcp;

/**
 * TODO: 
 * - Remove closed connections from m_connection
 * - Better event system (need a 1 to many (server -> clients) and many to 1 (clients -> server))
 * - Maybe template this? T and TPacked where T must be X with y functions
 */

class tcp_server
{
public:
    tcp_server() : m_acceptor(m_io_context) {
    }
    
    void start_listening(const tcp& protocol, uint16_t port) {
        m_acceptor.open(protocol);
        m_acceptor.bind(tcp::endpoint(protocol, port));
        m_acceptor.listen();
        
        start_accept();

        m_io_context.run();
    }

    void stop_listening() {
        m_acceptor.close();
    }

    void toggle_read_from_client(bool enable) {
        for (auto [_, connection] : m_connections) {
            connection->toggle_read_from_client(enable);
        }
    }

    void send_to_client_by(size_t connection_id, const entity_t& entity) {
        if (m_connections.contains(connection_id)) {
            m_connections.at(connection_id)->send_to_client(entity);
        }
        else {
            std::cout << "Connection ID not found: " << connection_id << std::endl;
        }
    }

    void set_on_accept_callback(std::function<void(size_t)>&& callback) {
        m_on_accept_callback = std::move(callback);
    }

    void set_on_receive_callback(std::function<void(const entity_t&)>&& callback) {
        m_on_receive_callback = std::move(callback);
    }

    void set_on_close_connection_callback(std::function<void(size_t)>&& callback) {
        m_on_connection_close_callback = std::move(callback);
    }

private:
    void start_accept() {
        tcp_connection::pointer new_connection = tcp_connection::create(
            m_io_context,
            &m_on_receive_callback,
            &m_on_connection_close_callback
        );

        m_acceptor.async_accept(
            new_connection->socket(), 
            std::bind(
                &tcp_server::handle_accept, 
                this, 
                new_connection,
                asio::placeholders::error
            )
        );
    }

    void handle_accept(tcp_connection::pointer new_connection, const std::error_code& error) {
        if (error) {
            std::cout << "handle_accept error: " + error.message() << std::endl;
            return; 
        }
        
        new_connection->assign_id(m_connection_count);
        m_connections.emplace(m_connection_count, new_connection);

        if (m_on_accept_callback) {
            m_on_accept_callback(m_connection_count);
        }

        m_connection_count++;

        std::cout << "tcp_server: Connection accepted - ESP: " << m_connection_count << std::endl;

        start_accept();
    }

private:
    tcp::acceptor m_acceptor;
    std::unordered_map<size_t, tcp_connection::pointer> m_connections;
    asio::io_context m_io_context;

    size_t m_connection_count = 0;
    std::function<void(size_t)> m_on_accept_callback;
    std::function<void(const entity_t&)> m_on_receive_callback;
    std::function<void(size_t)> m_on_connection_close_callback;
};