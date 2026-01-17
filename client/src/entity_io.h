#pragma once

#include "entity.h"
#include "net_status.h"

net_status_t send_entity(int socket, entity_t entity);
net_status_t recv_entity(int socket, entity_t* out_entity);