#ifndef SCALABLE_SERVER_HANDLE_UI_H
#define SCALABLE_SERVER_HANDLE_UI_H

#include "protocol_util.h"

#define MAX_SIZE 2048

struct request * get_type_and_object(struct server_options * options, char * request);
void handle_ui_request(struct server_options * options, struct request * request);
void clear_debug_file_buffer(FILE * debug_log_file);

#endif //SCALABLE_SERVER_HANDLE_UI_H
