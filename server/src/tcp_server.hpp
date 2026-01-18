#pragma once

#include <unordered_map>
#include <iostream> // prolly want better logging
#include <cstdint>
#include <functional>

#include <asio.hpp>

#include "tcp_connection.hpp"
#include "tcp_observer.hpp"

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
    tcp_server(tcp_observer_base& observer) : m_acceptor(m_io_context), m_observer(observer) {
        m_observer.set_tcp_server(this);
    }
    
    void start_listening(const tcp& protocol, uint16_t port) {
        m_acceptor.open(protocol);
        m_acceptor.set_option(tcp::acceptor::reuse_address(true));
        m_acceptor.bind(tcp::endpoint(protocol, port));
        m_acceptor.listen();
        
        std::cout << "TCP Server: Starting to listen for clients\n";

        start_accept();

        m_io_context.run();
    }

    void stop_listening() {
        m_acceptor.close();
    }

    void toggle_read_from_client(bool enable) {
        // tell every client that the server doesn't/does want to receive 
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

private:
    void start_accept() {
        tcp_connection::pointer new_connection = tcp_connection::create(m_io_context, m_observer);

        std::cout << "TCP Server: Waiting for more clients\n";

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
        
        // setup client and keep a ref of it
        new_connection->assign_id(m_connection_count);
        new_connection->toggle_read_from_client(true);
        m_connections.emplace(m_connection_count, new_connection);

        // notify of connection
        m_observer.on_client_connect(m_connection_count);

        // increment for next client id
        m_connection_count++;

        std::cout << "TCP Server: Connection accepted - ESP: " << m_connection_count << std::endl;

        // check for more client connections
        start_accept();
    }

private:
    asio::io_context m_io_context;
    tcp::acceptor m_acceptor;

    std::unordered_map<size_t, tcp_connection::pointer> m_connections;

    size_t m_connection_count = 0;
    tcp_observer_base& m_observer;
};