#ifndef SCALABLE_SERVER_HANDLE_SERVER_H
#define SCALABLE_SERVER_HANDLE_SERVER_H

#include "protocol_util.h"

void handle_server_request(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);

#endif //SCALABLE_SERVER_HANDLE_SERVER_H
