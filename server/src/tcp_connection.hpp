#pragma once

#include <functional>
#include <iostream>
#include <deque>
#include <array>

#include <asio.hpp>

#include "entity.h"
#include "logger.hpp"
#include "tcp_client_observer.hpp"

using asio::ip::tcp;

class tcp_connection : public std::enable_shared_from_this<tcp_connection>
{
private:
    typedef std::array<uint8_t, sizeof(net_entity_t)> entity_buffer;

public:
    typedef std::shared_ptr<tcp_connection> pointer;

    static pointer create(asio::io_context& io_context, tcp_client_observer_base& observer, std::function<void(uint16_t)>&& notify_server_disconnection) {
        return pointer(new tcp_connection(io_context, observer, std::move(notify_server_disconnection)));
    }

    tcp::socket& socket() {
        return m_socket;
    }

    void assign_id(uint16_t id) {
        m_connection_id = id;
    }

    void toggle_read_from_client(bool enable) {
        m_is_client_send_allowed = enable;

        if (enable) {
            read_from();
        }
    }

    void send_to_client(const entity_t& entity) {
        // add entity to sending queue
        m_write_queue.emplace_back(host_to_network_entity(entity));

        // prevent over writing by doing 1 operation at a time
        if (!m_is_writing) {
            write_to();
        }
    }

    void close_connection() {
        if (!m_socket.is_open()) return;
        
        m_socket.cancel();
        m_socket.close();

        m_observer.on_client_disconnect(m_connection_id);
        m_notify_server_disconnection(m_connection_id);
    }

private:
    tcp_connection(asio::io_context& io_context, tcp_client_observer_base& observer, std::function<void(uint16_t)>&& notify_server_disconnection) : 
        m_socket(io_context), m_observer(observer), m_notify_server_disconnection(std::move(notify_server_disconnection)) {
    }

    void write_to() {
        // prevent multiple requests from happening
        if (m_is_writing || m_write_queue.empty()) {
            return;
        }

        // get the current entity to send
        m_is_writing = true;
        const net_entity_t& net_entity = m_write_queue.front();

        // copy packed data to a buffer and send
        std::memcpy(m_write_data.data(), &net_entity, sizeof(net_entity_t)); 
        
        asio::async_write(
            m_socket, 
            asio::buffer(m_write_data),
            std::bind(
                &tcp_connection::handle_write, 
                shared_from_this(),
                asio::placeholders::error
            )
        );
    }

    void read_from() {    
        // prevent clients from senting information to server
        if (m_is_reading || !m_is_client_send_allowed) return;

        m_is_reading = true;

        asio::async_read(
            m_socket,
            asio::buffer(m_read_data),
            std::bind(
                &tcp_connection::handle_read, 
                shared_from_this(),
                asio::placeholders::error
            )
        );
    }

    void handle_write(const std::error_code& error) {
        if (!m_socket.is_open()) return; // prevent unwanted errors/writes if socket closed

        if (error) {
            LOG_WARN("Failed to send: {}", error.message());

            close_connection();

            return;
        }

        m_write_queue.pop_front();
        m_is_writing = false;

        write_to();
    }

    void handle_read(const std::error_code& error) {
        if (!m_socket.is_open()) return; // prevent unwanted errors/reads if socket closed

        if (error) {
            LOG_WARN("Failed to read: {}", error.message());

            close_connection();

            return;
        }

        net_entity_t net_entity;

        std::memcpy(&net_entity, m_read_data.data(), sizeof(net_entity_t));

        m_observer.on_receive_from(network_to_host_entity(net_entity));
        m_is_reading = false;

        read_from();
    }

private:
    tcp::socket m_socket;

    std::deque<net_entity_t> m_write_queue;
    entity_buffer m_write_data;
    entity_buffer m_read_data;

    uint16_t m_connection_id = 0;
    tcp_client_observer_base& m_observer;
    std::function<void(uint16_t)> m_notify_server_disconnection;

    bool m_is_client_send_allowed = true;
    bool m_is_writing = false;
    bool m_is_reading = false;
};