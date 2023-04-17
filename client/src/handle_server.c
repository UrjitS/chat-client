#include "handle_server.h"
#include <dc_c/dc_stdio.h>
#include <dc_c/dc_string.h>
#include <dc_posix/dc_unistd.h>
#include <dlfcn.h>
#include <dc_util/types.h>
#include <dc_posix/dc_string.h>

#define BASE 10

void handle_server_ping_user(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_ping_channel(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);

void handle_server_create(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_read(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_update(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_delete(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);

void handle_create_user_response(struct server_options *options, char *body);
void handle_create_channel_response(struct server_options *options, char *body);
void handle_create_message_response(struct server_options *options, char *body);
void handle_create_auth_response(struct server_options *options, char *body);

void handle_read_user_response(struct server_options *options, char *body);
void handle_read_channel_response(struct server_options *options, char *body);
void handle_read_message_response(struct server_options *options, char *body);
void handle_read_auth_response(struct server_options *options, char *body);


void handle_update_user_response(struct server_options *options, char *body);
void handle_update_channel_response(struct server_options *options, char *body);
void handle_update_message_response(struct server_options *options, char *body);
void handle_update_auth_response(struct server_options *options, char *body);

void handle_delete_user_response(struct server_options *options, char *body);
void handle_delete_channel_response(struct server_options *options, char *body);
void handle_delete_message_response(struct server_options *options, char *body);
void handle_delete_auth_response(struct server_options *options, char *body);

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
        case PINGUSER:
            handle_server_ping_user(options, binaryHeaderField, body);
            break;
        case PINGCHANNEL:
            handle_server_ping_channel(options, binaryHeaderField, body);
            break;
        default:
            break;
    }
}

/**
 * Handle PING STUFF
 */

void handle_server_ping_user(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body) {
    // The Ping Body of a User Ping must include the Display Name of the User that has been updated.

}

void handle_server_ping_channel(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body) {
    //  The Ping Body of the Channel Ping must include the Channel Name of the Channel that has been updated.

}

/**
 * Handle CREATE STUFF
 */

void handle_create_user_response(struct server_options *options, char *body)
{
    // 400 409 201

    fprintf(options->debug_log_file, "HANDLING CREATE USER RESP\n");
    clear_debug_file_buffer(options->debug_log_file);

    char * response_code = dc_strtok(options->env, body, "\3");

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
        write(STDOUT_FILENO, "OK\n", dc_strlen(options->env, "OK\n"));
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
        char buffer[DEFAULT_SIZE];
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
            dc_strcat(options->env, buffer, "\2");
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

    if (dc_strcmp(options->env, response_code, "400") == 0)
    {
        fprintf(options->debug_log_file, "Fields are invalid\n\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Fields are invalid\n", dc_strlen(options->env, "Fields are invalid\n"));
    } else if (dc_strcmp(options->env, response_code, "404") == 0)
    {
        fprintf(options->debug_log_file, "User account not found\n\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "User account not found\n", dc_strlen(options->env, "User account not found\n"));
    } else if (dc_strcmp(options->env, response_code, "403") == 0)
    {
        fprintf(options->debug_log_file, "Name does not match\n\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Sender name does Not match Display Name\n", dc_strlen(options->env, "Sender name does Not match Display Name\n"));
    } else if (dc_strcmp(options->env, response_code, "409") == 0)
    {
        fprintf(options->debug_log_file, "Channel Name Not UNIQUE\n\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Channel Name Not UNIQUE\n", dc_strlen(options->env, "Channel Name Not UNIQUE\n"));
    } else if (dc_strcmp(options->env, response_code, "201") == 0)
    {
        fprintf(options->debug_log_file, "CREATE CHANNEL SUCCESS\n\n");
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
    char * dup_body = dc_strdup(options->env, options->err, body);
    fprintf(options->debug_log_file, "HANDLING CREATE MESSAGE RESP\n");
    clear_debug_file_buffer(options->debug_log_file);

    char * response_code = dc_strtok(options->env, body, "\3");

    if (dc_strcmp(options->env, response_code, "400") == 0)
    {
        fprintf(options->debug_log_file, "Fields are invalid\n\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Fields are invalid\n", dc_strlen(options->env, "Fields are invalid\n"));
    } else if (dc_strcmp(options->env, response_code, "404") == 0)
    {
        fprintf(options->debug_log_file, "CHANNEL NOT FOUND\n\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "CHANNEL NOT FOUND\n", dc_strlen(options->env, "CHANNEL NOT FOUND\n"));
    } else if (dc_strcmp(options->env, response_code, "403") == 0)
    {
        fprintf(options->debug_log_file, "DISPLAY NAMES DONT MATCH\n\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "DISPLAY NAMES DONT MATCH\n", dc_strlen(options->env, "DISPLAY NAMES DONT MATCH\n"));
    } else if (dc_strcmp(options->env, response_code, "201") == 0)
    {
        fprintf(options->debug_log_file, "CREATE MESSAGE SUCCESS\n\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "OK2\n", dc_strlen(options->env, "OK\n"));
    }  else
    {
        fprintf(options->debug_log_file, "\nServer sent message\n");
        fprintf(options->debug_log_file, "Body %s\n", dup_body);
        clear_debug_file_buffer(options->debug_log_file);
        // Get semaphore

        char buffer[DEFAULT_SIZE];
        // It's  a message to be displayed on the UI
        // display-name ETX channel-name ETX message-content ETX timestamp ETX
        char * display_name = dc_strtok(options->env, NULL, "\3");
        dc_strtok(options->env, NULL, "\3");
        char * message_content = dc_strtok(options->env, NULL, "\3");
        char * time_stamp = dc_strtok(options->env, NULL, "\3");

        // Create the body
        dc_strcpy(options->env, buffer, display_name);
        dc_strcat(options->env, buffer, "\3");
        dc_strcat(options->env, buffer, message_content);
        dc_strcat(options->env, buffer, "\3");
        dc_strcat(options->env, buffer, time_stamp);
        dc_strcat(options->env, buffer, "\3");
        dc_strcat(options->env, buffer, "\0");

        fprintf(options->debug_log_file, "Message Received %s\n\n", buffer);
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, buffer, dc_strlen(options->env, buffer));

        // Release Semaphore
        sem_post(options->messaging_semaphore);
        fprintf(options->debug_log_file, "Sent to uI\n\n");
        // send CM request with 200
        send_create_message(options->env, options->err, options->socket_fd,"200\3");
        clear_debug_file_buffer(options->debug_log_file);
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

/**
 * READ STUFF
 */

void handle_read_user_response(struct server_options *options, char *body) {

}

void handle_read_channel_response(struct server_options *options, char *body) {

}

void handle_read_message_response(struct server_options *options, char *body) {
    // 400 404 200 206

    fprintf(options->debug_log_file, "HANDLING READ MESSAGE RESP\n");
    clear_debug_file_buffer(options->debug_log_file);

    char * response_code = dc_strtok(options->env, body, "\3");

    if (dc_strcmp(options->env, response_code, "400") == 0) {
        fprintf(options->debug_log_file, "Fields are invalid\n\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Fields are invalid\n", dc_strlen(options->env, "Fields are invalid\n"));
    } else if (dc_strcmp(options->env, response_code, "404") == 0) {
        fprintf(options->debug_log_file, "Channel NOT FOUND\n\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Channel NOT FOUND\n", dc_strlen(options->env, "Channel NOT FOUND\n"));
    } else if ((dc_strcmp(options->env, response_code, "200") == 0) || (dc_strcmp(options->env, response_code, "206") == 0)) {
        fprintf(options->debug_log_file, "Received All Counts\n");
        clear_debug_file_buffer(options->debug_log_file);

        char buffer[DEFAULT_SIZE];
        // read-message-partial-res-body = “206” ETX message-list
        char * message_size = dc_strtok(options->env, NULL, "\3");

        fprintf(options->debug_log_file, "CREATE USER AUTH SUCCESS\n");
        fprintf(options->debug_log_file, "Message NUMBER: %s\n", message_size);
        clear_debug_file_buffer(options->debug_log_file);

        uint16_t channel_size = dc_uint16_from_str(options->env, options->err, message_size, BASE);
        dc_strcpy(options->env, buffer, "OK ");

        for (int i = 0; i < channel_size; i++)
        {
            // message-info = display-name ETX channel-name ETX message-content ETX timestamp ETX
            dc_strcat(options->env, buffer, dc_strtok(options->env, NULL, "\3"));
            dc_strcat(options->env, buffer, " ");
            dc_strcat(options->env, buffer, dc_strtok(options->env, NULL, "\3"));
            dc_strcat(options->env, buffer, " ");
            dc_strcat(options->env, buffer, dc_strtok(options->env, NULL, "\3"));
            dc_strcat(options->env, buffer, " ");
            dc_strcat(options->env, buffer, dc_strtok(options->env, NULL, "\3"));
        }

        dc_strcat(options->env, buffer, "\0");
        fprintf(options->debug_log_file, "UI RESPONSE: %s\n\n", buffer);
        clear_debug_file_buffer(options->debug_log_file);

        write(STDOUT_FILENO, buffer, dc_strlen(options->env, buffer));
    } else {
        fprintf(options->debug_log_file, "INCORRECT RESPONSE CODE\n\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "SERVER ERROR\n", dc_strlen(options->env, "SERVER ERROR\n"));
    }
}

void handle_read_auth_response(struct server_options *options, char *body) {

}

void handle_server_read(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body) {
    switch (binaryHeaderField->object)
    {
        case USER:
            handle_read_user_response(options, body);
            break;
        case CHANNEL:
            handle_read_channel_response(options, body);
            break;
        case MESSAGE:
            handle_read_message_response(options, body);
            break;
        case AUTH:
            handle_read_auth_response(options, body);
            break;
        default:
            break;
    }
}

/**
 * UPDATE STUFF
 */

void handle_update_user_response(struct server_options *options, char *body) {

}

void handle_update_channel_response(struct server_options *options, char *body) {
    // 400 404 403 200
    // JOIN or LEAVE
    fprintf(options->debug_log_file, "HANDLING UPDATE CHANNEL RESP\n");
    clear_debug_file_buffer(options->debug_log_file);

    char * response_code = dc_strtok(options->env, body, "\3");

    if (dc_strcmp(options->env, response_code, "400") == 0)
    {
        fprintf(options->debug_log_file, "Fields are invalid\n\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Fields are invalid\n", dc_strlen(options->env, "Fields are invalid\n"));
    } else if (dc_strcmp(options->env, response_code, "404") == 0)
    {
        fprintf(options->debug_log_file, "Channel or User Does not Exist\n\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Channel Does not Exist\n", dc_strlen(options->env, "Channel Does not Exist\n"));
    } else if (dc_strcmp(options->env, response_code, "403") == 0)
    {
        fprintf(options->debug_log_file, "Sender Name Does NOT match Display Name\n\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Sender Name Does NOT match Display Name\n", dc_strlen(options->env, "Sender Name Does NOT match Display Name\n"));
    } else if (dc_strcmp(options->env, response_code, "200") == 0)
    {
        fprintf(options->debug_log_file, "UPDATE CHANNEL SUCCESS\n\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "OK\n", dc_strlen(options->env, "OK\n"));
    } else
    {
        fprintf(options->debug_log_file, "INCORRECT RESPONSE CODE\n\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "SERVER ERROR\n", dc_strlen(options->env, "SERVER ERROR\n"));
    }
}

void handle_update_message_response(struct server_options *options, char *body) {

}

void handle_update_auth_response(struct server_options *options, char *body) {

}


void handle_server_update(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body) {
    switch (binaryHeaderField->object)
    {
        case USER:
            handle_update_user_response(options, body);
            break;
        case CHANNEL:
            handle_update_channel_response(options, body);
            break;
        case MESSAGE:
            handle_update_message_response(options, body);
            break;
        case AUTH:
            handle_update_auth_response(options, body);
            break;
        default:
            break;
    }
}

/**
 * DELETE STUFF
 */

void handle_delete_user_response(struct server_options *options, char *body) {

}

void handle_delete_channel_response(struct server_options *options, char *body) {

}

void handle_delete_message_response(struct server_options *options, char *body) {

}

void handle_delete_auth_response(struct server_options *options, char *body) {
    // 400 404 403 412 200

    fprintf(options->debug_log_file, "HANDLING CREATE MESSAGE RESP\n");
    clear_debug_file_buffer(options->debug_log_file);

    char * response_code = dc_strtok(options->env, body, "\3");

    if (dc_strcmp(options->env, response_code, "400") == 0)
    {
        fprintf(options->debug_log_file, "Fields are invalid\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Fields are invalid\n", dc_strlen(options->env, "Fields are invalid\n"));
    } else if (dc_strcmp(options->env, response_code, "404") == 0)
    {
        fprintf(options->debug_log_file, "USER NOT FOUND\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "USER NOT FOUND\n", dc_strlen(options->env, "USER NOT FOUND\n"));
    } else if (dc_strcmp(options->env, response_code, "403") == 0)
    {
        fprintf(options->debug_log_file, "DISPLAY NAMES DONT MATCH\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "DISPLAY NAMES DONT MATCH\n", dc_strlen(options->env, "DISPLAY NAMES DONT MATCH\n"));
    } else if (dc_strcmp(options->env, response_code, "412") == 0)
    {
        fprintf(options->debug_log_file, "Already Exited\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "Already Exited\n", dc_strlen(options->env, "Already Exited\n"));
    } else if (dc_strcmp(options->env, response_code, "200") == 0)
    {
        fprintf(options->debug_log_file, "Destroy Auth SUCCESS\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "OK\n", dc_strlen(options->env, "OK\n"));
    }  else
    {
        fprintf(options->debug_log_file, "INCORRECT RESPONSE CODE\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "SERVER ERROR\n", dc_strlen(options->env, "SERVER ERROR\n"));
    }
}


void handle_server_delete(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body) {
    switch (binaryHeaderField->object)
    {
        case USER:
            handle_delete_user_response(options, body);
            break;
        case CHANNEL:
            handle_delete_channel_response(options, body);
            break;
        case MESSAGE:
            handle_delete_message_response(options, body);
            break;
        case AUTH:
            handle_delete_auth_response(options, body);
            break;
        default:
            break;
    }
}
