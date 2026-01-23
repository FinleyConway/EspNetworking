#pragma once

#include <functional>
#include <iostream>
#include <deque>
#include <array>

#include <asio.hpp>

#include "esp_info.h"
#include "tcp_logger.hpp"
#include "tcp_client_observer.hpp"

using asio::ip::tcp;

class tcp_connection : public std::enable_shared_from_this<tcp_connection>
{
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

    void send_to_client(const esp_info_t& esp_info) {
        if (m_is_closing) {
            LOG_WARN("Client {} is disconnecting, write prevented", m_connection_id);
            return;
        }

        // add esp_info to sending queue
        m_write_queue.emplace_back(esp_info_host_to_network(esp_info));

        // prevent over writing by doing 1 operation at a time
        if (!m_is_writing) {
            write_to();
        }
    }

    void close_connection() {
        if (m_is_closing) return;

        // make sure everything stops
        m_is_closing = true;
        m_is_reading = false;
        m_is_writing = false;
        m_write_queue.clear();  

        if (std::error_code ec; m_socket.close(ec)) {
            LOG_ERROR("Client {}, socket failed to close: {}", m_connection_id, ec.message());
        }

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

        m_is_writing = true;

        asio::async_write(
            m_socket, 
            asio::buffer(&m_write_queue.front(), sizeof(net_esp_info_t)), // queue elements doesnt get invalidated so this is fine
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
        m_is_writing = false;

        if (error) {
            handle_io_errors(error);
            return;
        }

        m_write_queue.pop_front();
        write_to();
    }

    void handle_read(const std::error_code& error) {
        m_is_reading = false;

        if (error) {
            handle_io_errors(error);
            return;
        }

        net_esp_info_t net_esp_info;
        std::memcpy(&net_esp_info, m_read_data.data(), sizeof(net_esp_info_t));

        m_observer.on_receive_from(esp_info_network_to_host(net_esp_info));
        read_from();
    }

    void handle_io_errors(const std::error_code& error) {
        if (m_is_closing)
            return;

        if (error == asio::error::operation_aborted) return;

        if (error == asio::error::eof || error == asio::error::connection_reset) {
            close_connection();
            return;
        }

        LOG_WARN("Client {} I/O error: {}", m_connection_id, error.message());

        close_connection();
    }

private:
    tcp::socket m_socket;

    std::deque<net_esp_info_t> m_write_queue;
    std::array<uint8_t, sizeof(net_esp_info_t)> m_read_data;

    uint16_t m_connection_id = 0;
    tcp_client_observer_base& m_observer;
    std::function<void(uint16_t)> m_notify_server_disconnection;

    bool m_is_client_send_allowed = true;
    bool m_is_writing = false;
    bool m_is_reading = false;
    bool m_is_closing = false;
};