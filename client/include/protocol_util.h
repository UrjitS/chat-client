#ifndef CHAT_SERVER_PROTOCOL_UTIL_H
#define CHAT_SERVER_PROTOCOL_UTIL_H

#include <dc_env/env.h>
#include <dc_error/error.h>
#include <arpa/inet.h>
#include <dc_util/networking.h>
#include <semaphore.h>

#define DEFAULT_SIZE 1024
#define DEFAULT_VERSION 0x1

enum Type {
    CREATE = 0x1,
    READ = 0x2,
    UPDATE = 0x3,
    DESTROY = 0x4,
    PINGUSER = 0x9,
    PINGCHANNEL = 0xa
};

enum Object {
    USER = 0x01,
    CHANNEL = 0x02,
    MESSAGE = 0x03,
    AUTH = 0x04
};

struct binary_header_field {
    unsigned int version : 4; // 4 bit version number
    unsigned int type : 4; // 4 bit type number
    uint8_t object; // 8 bit object type
    uint16_t body_size; // 16 bit body size
};

struct request{
    char * type;
    char * obj;
    char * data;
};

struct server_options{
    struct dc_env *env;
    struct dc_error *err;

    FILE * debug_log_file;

    int socket_fd;
    sem_t * messaging_semaphore;
};
/**
 * Display the header information to stdout.
 * @param header Binary header struct
 * @param data
 */
struct binary_header_field * deserialize_header(struct dc_env *env, struct dc_error *err, int fd, uint32_t value);
void serialize_header(struct dc_env *env, struct dc_error *err, struct binary_header_field * header, int fd,
                      const char * body);
void clear_debug_file_buffer(FILE * debug_log_file);
/**
 * Send Create Stuff
 */
void send_create_user(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_create_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_create_message(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_create_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body);

/**
 * Send Read Stuff
 */
__attribute__((unused)) void send_read_user(struct dc_env *env, struct dc_error *err, int fd, const char * body);

__attribute__((unused)) void send_read_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_read_message(struct dc_env *env, struct dc_error *err, int fd, const char * body);

/**
 * Send Update Stuff
 */
__attribute__((unused)) void send_update_user(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_update_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body);

__attribute__((unused)) void send_update_message(struct dc_env *env, struct dc_error *err, int fd, const char * body);

__attribute__((unused)) void send_update_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body);

/**
 * Send Delete Stuff
 */
__attribute__((unused)) void send_delete_user(struct dc_env *env, struct dc_error *err, int fd, const char * body);

__attribute__((unused)) void send_delete_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body);

__attribute__((unused)) void send_delete_message(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_delete_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body);

#endif //CHAT_SERVER_PROTOCOL_UTIL_H
