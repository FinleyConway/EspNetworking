#pragma once

#include <iostream>
#include <deque>
#include <array>

#include <asio.hpp>

#include "entity.h"

using asio::ip::tcp;

class tcp_connection : public std::enable_shared_from_this<tcp_connection>
{
private:
    typedef std::array<uint8_t, sizeof(net_entity_t)> entity_buffer;

public:
    typedef std::shared_ptr<tcp_connection> pointer;

    static pointer create(asio::io_context& io_context) {
        return pointer(new tcp_connection(io_context));
    }

    tcp::socket& socket() {
        return m_socket;
    }

    void send(const entity_t& entity) {
        // add entity to sending queue
        m_write_queue.emplace_back(host_to_network_entity(entity));

        // prevent over writing by doing 1 operation at a time
        if (!m_is_writing) {
            write_to();
        }
    }

private:
    tcp_connection(asio::io_context& io_context) : m_socket(io_context) {}

    void write_to() {
        if (m_write_queue.empty()) {
            m_is_writing = false;

            return;
        }

        // get the current entity to send
        m_is_writing = true;
        const net_entity_t& net_entity = m_write_queue.front();

        // copy packed data to a buffer and send
        std::memcpy(m_data.data(), &net_entity, sizeof(net_entity_t)); 
        
        asio::async_write(
            m_socket, 
            asio::buffer(m_data),
            std::bind(
                &tcp_connection::handle_write, 
                shared_from_this(),
                asio::placeholders::error
            )
        );
    }

    void handle_write(const std::error_code& error) {
        if (error) {
            std::cerr << "Write failed: " << error.message() << "\n";
            m_socket.close();

            return;
        }

        m_write_queue.pop_front();

        write_to();
    }

private:
    bool m_is_writing = false;
    tcp::socket m_socket;
    std::deque<net_entity_t> m_write_queue;
    entity_buffer m_data;
};