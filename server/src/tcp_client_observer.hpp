#pragma once

#include <cstddef>
#include <cassert>

#include "entity.h"

class tcp_server;

class tcp_client_observer_base 
{
public:
    virtual ~tcp_client_observer_base() = default;

    virtual void on_client_connect(uint16_t client_id) = 0;
    virtual void on_receive_from(const entity_t& entity) = 0;
    virtual void on_client_disconnect(uint16_t client_id) = 0;

protected:
    tcp_server* get_tcp_server() const {
        assert(m_tcp_server != nullptr && "tcp_server is null!");

        return m_tcp_server;
    } 

private:
    friend tcp_server;

    void set_tcp_server(tcp_server* server) {
        m_tcp_server = server;
    }

private:
    tcp_server* m_tcp_server = nullptr;
};