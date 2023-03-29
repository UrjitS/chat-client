#include "handle_server.h"
#include <dc_c/dc_stdio.h>
#include <dc_c/dc_stdlib.h>
#include <dc_c/dc_string.h>
#include <dc_env/env.h>
#include <dc_error/error.h>
#include <dc_posix/dc_unistd.h>
#include <dc_util/io.h>
#include <dlfcn.h>
#include <dc_posix/dc_string.h>
#include <dc_util/system.h>
#include <dc_c/dc_stdio.h>
#include <dc_c/dc_stdlib.h>
#include <dc_c/dc_string.h>
#include <dc_posix/arpa/dc_inet.h>
#include <dc_posix/dc_dlfcn.h>
#include <dc_posix/dc_poll.h>
#include <dc_posix/dc_semaphore.h>
#include <dc_posix/dc_signal.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_unistd.h>
#include <dc_posix/sys/dc_select.h>
#include <dc_posix/sys/dc_socket.h>
#include <dc_posix/sys/dc_wait.h>
#include <dc_unix/dc_getopt.h>
#include <dc_util/networking.h>
#include <dc_util/system.h>
#include <dc_util/types.h>

#define BASE 10

void handle_server_create(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_read(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_update(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_delete(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);

void handle_create_user_response(struct server_options *options, char *body);
void handle_create_channel_response(struct server_options *options, char *body);
void handle_create_message_response(struct server_options *options, char *body);
void handle_create_auth_response(struct server_options *options, char *body);


void handle_server_request(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body) {
    switch (binaryHeaderField->type)
    {
        case CREATE:
            handle_server_create(options, binaryHeaderField, body);
            break;
        case READ:
            handle_server_read(options, binaryHeaderField, body);
            break;
        case UPDATE:
            handle_server_update(options, binaryHeaderField, body);
            break;
        case DESTROY:
            handle_server_delete(options, binaryHeaderField, body);
            break;
        default:
            break;
    }
}

void handle_create_user_response(struct server_options *options, char *body)
{
    // 400 409 201

    fprintf(options->debug_log_file, "HANDLING CREATE USER RESP\n");
    clear_debug_file_buffer(options->debug_log_file);

    char * response_code = dc_strtok(options->env, body, "\3");
    response_code[3] = '\0';

    if (dc_strcmp(options->env, response_code, "400") == 0) {
        fprintf(options->debug_log_file, "Fields are invalid\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Fields are invalid\n", dc_strlen(options->env, "Fields are invalid\n"));
    } else if (dc_strcmp(options->env, response_code, "409") == 0) {
        fprintf(options->debug_log_file, "NON UNIQUE CREDENTIALS\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Username or email already exists\n", dc_strlen(options->env, "Username or email already exists\n"));
    } else if (dc_strcmp(options->env, response_code, "201") == 0) {
        fprintf(options->debug_log_file, "CREATE USER SUCCESS\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "OK", dc_strlen(options->env, "OK"));
    } else {
        fprintf(options->debug_log_file, "INCORRECT RESPONSE CODE\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "SERVER ERROR\n", dc_strlen(options->env, "SERVER ERROR\n"));
    }
}

void handle_create_auth_response(struct server_options *options, char *body)
{
    // 400 403 200
    fprintf(options->debug_log_file, "HANDLING CREATE USER AUTH RESP\n");
    clear_debug_file_buffer(options->debug_log_file);

    char * response_code = dc_strtok(options->env, body, "\3");
    response_code[3] = '\0';

    if (dc_strcmp(options->env, response_code, "400") == 0)
    {
        fprintf(options->debug_log_file, "Fields are invalid\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Fields are invalid\n", dc_strlen(options->env, "Fields are invalid\n"));
    } else if (dc_strcmp(options->env, response_code, "403") == 0)
    {
        fprintf(options->debug_log_file, "User account not found\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "User account not found\n", dc_strlen(options->env, "User account not found\n"));
    } else if (dc_strcmp(options->env, response_code, "200") == 0)
    {
        char buffer[1024];
        // “200” ETX display-name ETX privilege-level ETX channel-name-list
        char * display_name = dc_strtok(options->env, NULL, "\3");
        char * privilege_level = dc_strtok(options->env, NULL, "\3");
        char * channel_name_list_size = dc_strtok(options->env, NULL, "\3");

        fprintf(options->debug_log_file, "CREATE USER AUTH SUCCESS\n");
        fprintf(options->debug_log_file, "DisplayName: %s\n", display_name);
        fprintf(options->debug_log_file, "Privy Level: %s\n", privilege_level);
        fprintf(options->debug_log_file, "CHANNEL NUMBER: %s\n", channel_name_list_size);
        clear_debug_file_buffer(options->debug_log_file);

        // OK GLOBAL, Channel1\0
        uint16_t channel_size = dc_uint16_from_str(options->env, options->err, channel_name_list_size, BASE);
        dc_strcpy(options->env, buffer, "OK ");

        for (int i = 0; i < channel_size; i++)
        {
            dc_strcat(options->env, buffer, dc_strtok(options->env, NULL, "\3"));
            dc_strcat(options->env, buffer, " ");
        }

        dc_strcat(options->env, buffer, "\0");
        fprintf(options->debug_log_file, "UI RESPONSE: %s\n", buffer);
        clear_debug_file_buffer(options->debug_log_file);

        write(STDOUT_FILENO, buffer, dc_strlen(options->env, buffer));
    } else {
        fprintf(options->debug_log_file, "INCORRECT RESPONSE CODE\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "SERVER ERROR\n", dc_strlen(options->env, "SERVER ERROR\n"));
    }
}

void handle_create_channel_response(struct server_options *options, char *body)
{
    // 400 404 403 409 201
    fprintf(options->debug_log_file, "HANDLING CREATE CHANNEL RESP\n");
    clear_debug_file_buffer(options->debug_log_file);

    char * response_code = dc_strtok(options->env, body, "\3");
    response_code[3] = '\0';

    if (dc_strcmp(options->env, response_code, "400") == 0)
    {
        fprintf(options->debug_log_file, "Fields are invalid\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Fields are invalid\n", dc_strlen(options->env, "Fields are invalid\n"));
    } else if (dc_strcmp(options->env, response_code, "404") == 0)
    {
        fprintf(options->debug_log_file, "User account not found\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "User account not found\n", dc_strlen(options->env, "User account not found\n"));
    } else if (dc_strcmp(options->env, response_code, "403") == 0)
    {
        fprintf(options->debug_log_file, "Name does not match\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Sender name does Not match Display Name\n", dc_strlen(options->env, "Sender name does Not match Display Name\n"));
    } else if (dc_strcmp(options->env, response_code, "409") == 0)
    {
        fprintf(options->debug_log_file, "Channel Name Not UNIQUE\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Channel Name Not UNIQUE\n", dc_strlen(options->env, "Channel Name Not UNIQUE\n"));
    } else if (dc_strcmp(options->env, response_code, "201") == 0)
    {
        fprintf(options->debug_log_file, "CREATE CHANNEL SUCCESS\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "OK\n", dc_strlen(options->env, "OK\n"));
    } else {
        fprintf(options->debug_log_file, "INCORRECT RESPONSE CODE\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "SERVER ERROR\n", dc_strlen(options->env, "SERVER ERROR\n"));
    }

}

void handle_create_message_response(struct server_options *options, char *body)
{
    // 400 404 403 201

    fprintf(options->debug_log_file, "HANDLING CREATE MESSAGE RESP\n");
    clear_debug_file_buffer(options->debug_log_file);

    char * response_code = dc_strtok(options->env, body, "\3");
    response_code[3] = '\0';

    if (dc_strcmp(options->env, response_code, "400") == 0)
    {
        fprintf(options->debug_log_file, "Fields are invalid\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Fields are invalid\n", dc_strlen(options->env, "Fields are invalid\n"));
    } else if (dc_strcmp(options->env, response_code, "404") == 0)
    {
        fprintf(options->debug_log_file, "CHANNEL NOT FOUND\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "CHANNEL NOT FOUND\n", dc_strlen(options->env, "CHANNEL NOT FOUND\n"));
    } else if (dc_strcmp(options->env, response_code, "403") == 0)
    {
        fprintf(options->debug_log_file, "DISPLAY NAMES DONT MATCH\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "DISPLAY NAMES DONT MATCH\n", dc_strlen(options->env, "DISPLAY NAMES DONT MATCH\n"));
    } else if (dc_strcmp(options->env, response_code, "201") == 0)
    {
        fprintf(options->debug_log_file, "CREATE MESSAGE SUCCESS\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "OK\n", dc_strlen(options->env, "OK"));
    }  else
    {
        fprintf(options->debug_log_file, "INCORRECT RESPONSE CODE\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "SERVER ERROR\n", dc_strlen(options->env, "SERVER ERROR\n"));
    }
}

void handle_server_create(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body) {
    switch (binaryHeaderField->object)
    {
        case USER:
            handle_create_user_response(options, body);
            break;
        case CHANNEL:
            handle_create_channel_response(options, body);
            break;
        case MESSAGE:
            handle_create_message_response(options, body);
            break;
        case AUTH:
            handle_create_auth_response(options, body);
            break;
        default:
            break;
    }
}

void handle_server_read(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body) {

}

void handle_server_update(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body) {

}

void handle_server_delete(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body) {

}
