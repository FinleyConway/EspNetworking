#pragma once

#include <asio.hpp>

using asio::ip::tcp;

class tcp_connection : public std::enable_shared_from_this<tcp_connection>
{
public:
    typedef std::shared_ptr<tcp_connection> pointer;

    static pointer create(asio::io_context& io_context) {
        return pointer(new tcp_connection(io_context));
    }

    tcp::socket& socket() {
        return m_socket;
    }

    void start() {
        m_message = "hello";

        asio::async_write(
            m_socket, 
            asio::buffer(m_message),
            std::bind(
                &tcp_connection::handle_write, 
                shared_from_this(),
                asio::placeholders::error,
                asio::placeholders::bytes_transferred
            )
        );
    }

private:
    tcp_connection(asio::io_context& io_context) : m_socket(io_context) {}

    void handle_write(const std::error_code& error, size_t bytes_trasferred) {
    }

private:
    tcp::socket m_socket;
    std::string m_message; // message as a member as you want to keep it alive until the asynchronous opration is done.
};