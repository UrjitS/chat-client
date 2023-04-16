#include "../include/protocol_util.h"
#include <dc_c/dc_stdio.h>
#include <dc_c/dc_stdlib.h>
#include <dc_c/dc_string.h>
#include <dc_env/env.h>
#include <dc_error/error.h>
#include <netinet/in.h>
#include <dc_util/io.h>

struct binary_header_field * deserialize_header(struct dc_env *env, struct dc_error *err, int fd, uint32_t value) {
    struct binary_header_field * header;
    uint32_t header2;

    // Convert to network byte order
    value = ntohl(value);

    header = malloc(sizeof(struct binary_header_field));
    header->version = (value >> 28) & 0x0F; // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    header->type = (value >> 24) & 0x0F;    // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    // Check if the type is any of the pings or not
    if (header->type == PINGUSER || header->type == PINGCHANNEL) {
        return header;
    }

    // read the remaining 3 bytes
    dc_read_fully(env, err, fd, &header2, 3);

    // Convert to network byte order
    header2 = ntohl(header2);

    header->object = (header2 >> 24) & 0xFF;  // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    header->body_size = (header2 >> 8) & 0xFFFF;     // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    return header;
}

void serialize_header(struct dc_env *env, struct dc_error *err, struct binary_header_field * header, int fd,
                      const char * body)
{
    char data[DEFAULT_SIZE];

    // Create the packet
    uint32_t packet = (((((uint32_t)header->version) & 0xF) << 28)) | ((((uint32_t)header->type) & 0xF) << 24) | // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
                      ((((uint32_t)header->object) & 0xFF) << 16) | (((uint32_t)header->body_size) & 0xFFFF);  // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    // Convert to network byte order.
    packet = htonl(packet);

    // Copy the packet into the data buffer
    dc_memcpy(env, data, &packet, sizeof(uint32_t));

    // Add the body to the data buffer
    dc_memcpy(env, data + sizeof(uint32_t), body, dc_strlen(env, body));

    dc_write_fully(env, err, fd, &data, (sizeof(uint32_t) + dc_strlen(env, body)));
}

void clear_debug_file_buffer(FILE * debug_log_file)
{
    fflush(debug_log_file);
    setbuf(debug_log_file, NULL);
}

/**
 * PAIN
 */

/**
 * Send Create Stuff
 */
void send_create_user(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create specific header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = CREATE;
    header.object = USER;
    header.body_size = dc_strlen(env, body);

    // Send the header
    serialize_header(env, err, &header, fd, body);
}

void send_create_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create specific header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = CREATE;
    header.object = CHANNEL;
    header.body_size = dc_strlen(env, body);
    // Send the header
    serialize_header(env, err, &header, fd, body);
}

void send_create_message(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create specific header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = CREATE;
    header.object = MESSAGE;
    header.body_size = dc_strlen(env, body);
    // Send the header
    serialize_header(env, err, &header, fd, body);
}

void send_create_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create specific header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = CREATE;
    header.object = AUTH;
    header.body_size = dc_strlen(env, body);
    // Send the header
    serialize_header(env, err, &header, fd, body);
}

/**
 * Send Read Stuff
 */
__attribute__((unused)) void send_read_user(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create specific header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = READ;
    header.object = USER;
    header.body_size = dc_strlen(env, body);
    // Send the header
    serialize_header(env, err, &header, fd, body);
}

__attribute__((unused)) void send_read_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create specific header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = READ;
    header.object = CHANNEL;
    header.body_size = dc_strlen(env, body);
    // Send the header
    serialize_header(env, err, &header, fd, body);
}

void send_read_message(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create specific header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = READ;
    header.object = MESSAGE;
    header.body_size = dc_strlen(env, body);
    // Send the header
    serialize_header(env, err, &header, fd, body);
}

/**
 * Send Update Stuff
 */
__attribute__((unused)) void send_update_user(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create specific header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = UPDATE;
    header.object = USER;
    header.body_size = dc_strlen(env, body);
    // Send the header
    serialize_header(env, err, &header, fd, body);
}

void send_update_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create specific header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = UPDATE;
    header.object = CHANNEL;
    header.body_size = dc_strlen(env, body);
    // Send the header
    serialize_header(env, err, &header, fd, body);
}

__attribute__((unused)) void send_update_message(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create specific header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = UPDATE;
    header.object = MESSAGE;
    header.body_size = dc_strlen(env, body);
    // Send the header
    serialize_header(env, err, &header, fd, body);
}

__attribute__((unused)) void send_update_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create specific header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = UPDATE;
    header.object = AUTH;
    header.body_size = dc_strlen(env, body);
    // Send the header
    serialize_header(env, err, &header, fd, body);
}

/**
 * Send Delete Stuff
 */
__attribute__((unused)) void send_delete_user(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create specific header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = DESTROY;
    header.object = USER;
    header.body_size = dc_strlen(env, body);
    // Send the header
    serialize_header(env, err, &header, fd, body);
}

__attribute__((unused)) void send_delete_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create specific header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = DESTROY;
    header.object = CHANNEL;
    header.body_size = dc_strlen(env, body);
    // Send the header
    serialize_header(env, err, &header, fd, body);
}

__attribute__((unused)) void send_delete_message(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create specific header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = DESTROY;
    header.object = MESSAGE;
    header.body_size = dc_strlen(env, body);
    // Send the header
    serialize_header(env, err, &header, fd, body);
}

void send_delete_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create specific header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = DESTROY;
    header.object = AUTH;
    header.body_size = dc_strlen(env, body);
    // Send the header
    serialize_header(env, err, &header, fd, body);
}
