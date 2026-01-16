#include <array>
#include <iostream>
#include <memory>

#include <asio.hpp>

using asio::ip::tcp;

// REF: https://think-async.com/Asio/asio-1.36.0/doc/asio/tutorial/tutdaytime3.html

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

    void handle_write(const std::error_code&, size_t) {
    }

private:
    tcp::socket m_socket;
    std::string m_message;
};

class tcp_server
{
public:
    tcp_server(asio::io_context& io_context) : 
        m_io_context(io_context),
        m_acceptor(io_context, tcp::endpoint(tcp::v4(), 8080))
    {
        start_accept();
    }

private:
    void start_accept() {
        tcp_connection::pointer new_connection = tcp_connection::create(m_io_context);

        m_acceptor.async_accept(
            new_connection->socket(), 
            std::bind(&tcp_server::handle_accept, this, new_connection,
            asio::placeholders::error
        ));
    }

    void handle_accept(tcp_connection::pointer new_connection, const std::error_code& error) {
        if (!error) {
            new_connection->start();
        }

        start_accept();
    }

private:
    asio::io_context& m_io_context;
    tcp::acceptor m_acceptor;
};

int main() {
    asio::io_context io_context;
    tcp_server server(io_context);

    io_context.run();

    return 0;
}