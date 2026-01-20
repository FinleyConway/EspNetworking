#pragma once

#include <unordered_map>
#include <functional>
#include <cstdint>
#include <cassert>

#include <asio.hpp>

#include "tcp_logger.hpp"
#include "tcp_connection.hpp"
#include "tcp_client_observer.hpp"

using asio::ip::tcp;

class tcp_server
{
public:
    tcp_server(tcp_client_observer_base& observer) : m_acceptor(m_io_context), m_observer(observer) {
        logger::init();
        m_observer.set_tcp_server(this);
    }
    
    void start_listening(const tcp& protocol, uint16_t port) {
        try {
            m_acceptor.open(protocol);
            m_acceptor.set_option(tcp::acceptor::reuse_address(true));
            m_acceptor.bind(tcp::endpoint(protocol, port));
            m_acceptor.listen();
        }
        catch (const std::exception &e) {
            LOG_ERROR("Failed to start accepting clients: {}", e.what());

            return;
        }
        
        LOG_INFO("Starting to accept new clients.");

        start_accept();

        m_io_context.run();
    }

    void stop_listening() {
        LOG_INFO("Stopped accepting clients.");

        m_acceptor.close();
    }

    void toggle_read_from_client(bool enable) {
        LOG_INFO("Listening to clients has {}.", enable ? "started" : "stopped");

        // tell every client that the server doesn't/does want to receive 
        for (auto [_, connection] : m_connections) {
            connection->toggle_read_from_client(enable);
        }
    }

    void send_to_client_by(uint16_t client_id, const esp_info_t& esp_info) {
        LOG_INFO("Sending to clients {}.", client_id);

        if (m_connections.contains(client_id)) {
            m_connections.at(client_id)->send_to_client(esp_info);
        }
        else {
            LOG_ERROR("Failed to send to client as client {} was not found!", client_id);
        }
    }

    void disconnect_client_by(uint16_t client_id) {
        if (m_connections.contains(client_id)) {
            m_connections.at(client_id)->close_connection();
        }
        else {
            LOG_ERROR("Failed to disconnect client as client {} was not found!", client_id);
        }
    }

    void close() {
        stop_listening();
        m_io_context.stop();

        LOG_INFO("Shutting down.");
    }

private:
    void start_accept() {
        tcp_connection::pointer new_connection = tcp_connection::create(
            m_io_context,
            m_observer,
            std::bind(&tcp_server::handle_client_disconnect, this, std::placeholders::_1)
        );

        LOG_INFO("Listening for clients connections...");

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
            LOG_WARN("Accepting incoming client failed: {}", error.message());
            return; 
        }
        
        // setup client and keep a ref of it
        new_connection->assign_id(m_connection_count);
        new_connection->toggle_read_from_client(true);
        m_connections.emplace(m_connection_count, new_connection);

        // notify of connection
        m_observer.on_client_connect(m_connection_count);

        LOG_INFO("Connection accepted: ESP {} connected", m_connection_count);

        // increment for next client id
        m_connection_count++;

        // check for more client connections
        start_accept();
    }

    void handle_client_disconnect(uint16_t client_id) {
        assert(m_connections.contains(client_id) && "Trying to handle a client that doesn't exist!\n");

        LOG_INFO("Client {} disconnected", client_id);

        m_connections.erase(client_id);
    }

private:
    asio::io_context m_io_context;
    tcp::acceptor m_acceptor;

    std::unordered_map<uint16_t, tcp_connection::pointer> m_connections;

    uint16_t m_connection_count = 1; // start at 1 to allow 0 to be error
    tcp_client_observer_base& m_observer;
};