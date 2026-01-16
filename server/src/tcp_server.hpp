#pragma once

#include <unordered_map>
#include <iostream> // prolly want better logging
#include <cstdint>

#include <asio.hpp>

#include "tcp_connection.hpp"

using asio::ip::tcp;

class tcp_server
{
public:
    tcp_server() : m_acceptor(m_io_context)
    {
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

private:
    void start_accept() {
        tcp_connection::pointer new_connection = tcp_connection::create(m_io_context);

        m_acceptor.async_accept(
            new_connection->socket(), 
            std::bind(
                &tcp_server::handle_accept, this, new_connection,
                asio::placeholders::error
            )
        );
    }

    void handle_accept(tcp_connection::pointer new_connection, const std::error_code& error) {
        if (error) {
            std::cout << "handle_accept error: " + error.message() << std::endl;
            return; 
        }

        new_connection->start();
        m_connections.emplace(m_connection_count, new_connection);
        m_connection_count++;

        std::cout << "tcp_server: Connection accepted\n";

        start_accept();
    }

private:
    size_t m_connection_count = 0;
    asio::io_context m_io_context;
    tcp::acceptor m_acceptor;
    std::unordered_map<size_t, tcp_connection::pointer> m_connections;
};