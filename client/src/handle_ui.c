#include "handle_ui.h"
#include <dc_c/dc_stdio.h>
#include <dc_c/dc_stdlib.h>
#include <dc_c/dc_string.h>
#include <dlfcn.h>
#include <dc_posix/dc_string.h>

void handle_create(struct server_options * options, struct request * request);
void handle_read(struct server_options * options, struct request * request);
void handle_update(struct server_options * options, struct request * request);
void handle_delete(struct server_options * options, struct request * request);

void handle_ui_request(struct server_options * options, struct request * request) {
    if (dc_strcmp(options->env, "CREATE", request->type) == 0) {
        handle_create(options, request);
    } else if (dc_strcmp(options->env, "READ", request->type) == 0) {
        handle_read(options, request);
    } else if (dc_strcmp(options->env, "UPDATE", request->type) == 0) {
        handle_update(options, request);
    } else if (dc_strcmp(options->env, "DELETE", request->type) == 0) {
        handle_delete(options, request);
    }
}

struct request * get_type_and_object(struct server_options * options, char * request)
{
    struct request * req = malloc(sizeof(struct request));
    req->data = dc_strdup(options->env, options->err, request);

    char * type = dc_strtok(options->env, request, " ");
    char * obj = dc_strtok(options->env, NULL, " ");

    req->type = dc_strdup(options->env, options->err, type);
    req->obj = dc_strdup(options->env, options->err, obj);

    fprintf(options->debug_log_file, "Type: %s\n", req->type);
    fprintf(options->debug_log_file, "Object: %s\n", req->obj);

    free(request);
    return req;
}

void handle_create(struct server_options * options, struct request * request) {
    char body[MAX_SIZE];
    if (dc_strcmp(options->env, "U", request->obj) == 0)
    {

        dc_strtok(options->env, request->data, " ");
        dc_strtok(options->env, NULL, " ");
        char * login_token = dc_strtok(options->env, NULL, " ");
        char * display_token = dc_strtok(options->env, NULL, " ");
        char * password = dc_strtok(options->env, NULL, " ");

        dc_strcpy(options->env, body, login_token);
        dc_strcat(options->env, body, "\3");
        dc_strcat(options->env, body, display_token);
        dc_strcat(options->env, body, "\3");
        dc_strcat(options->env, body, password);
        dc_strcat(options->env, body, "\3");

        fprintf(options->debug_log_file, "Body %s\n", body); // Write a string to the file
        clear_debug_file_buffer(options->debug_log_file);
        send_create_user(options->env, options->err, options->socket_fd, body);

    } else if (dc_strcmp(options->env, "C", request->obj) == 0)
    {
        // channel-name ETX display-name ETX publicity ETX [if private then password ETX]
        dc_strtok(options->env, request->data, " ");
        dc_strtok(options->env, NULL, " ");
        char * channel_name = dc_strtok(options->env, NULL, " ");
        char * display_name = dc_strtok(options->env, NULL, " ");
        char * publicity = dc_strtok(options->env, NULL, " ");

        dc_strcpy(options->env, body, channel_name);
        dc_strcat(options->env, body, "\3");
        dc_strcat(options->env, body, display_name);
        dc_strcat(options->env, body, "\3");
        dc_strcat(options->env, body, publicity);
        dc_strcat(options->env, body, "\3");

        fprintf(options->debug_log_file, "Body %s\n", body); // Write a string to the file
        clear_debug_file_buffer(options->debug_log_file);
        send_create_channel(options->env, options->err, options->socket_fd,request->data);
    } else if (dc_strcmp(options->env, "M", request->obj) == 0)
    {
        // display-name ETX channel-name ETX message-content ETX timestamp ETX
        char time_stamp[100];
        sprintf(time_stamp, "%016lx\3", time(NULL));

        dc_strtok(options->env, request->data, " ");
        dc_strtok(options->env, NULL, " ");
        char * display_name = dc_strtok(options->env, NULL, " ");
        char * channel_name = dc_strtok(options->env, NULL, " ");
        char * message_content = dc_strtok(options->env, NULL, " ");

        dc_strcpy(options->env, body, display_name);
        dc_strcat(options->env, body, "\3");
        dc_strcat(options->env, body, channel_name);
        dc_strcat(options->env, body, "\3");
        dc_strcat(options->env, body, message_content);
        dc_strcat(options->env, body, "\3");
        dc_strcat(options->env, body, time_stamp);

        fprintf(options->debug_log_file, "Body %s\n", body); // Write a string to the file
        clear_debug_file_buffer(options->debug_log_file);
        send_create_message(options->env, options->err, options->socket_fd,body);
    } else if (dc_strcmp(options->env, "A", request->obj) == 0)
    {
        dc_strtok(options->env, request->data, " ");
        dc_strtok(options->env, NULL, " ");
        char * login_token = dc_strtok(options->env, NULL, " ");
        char * password = dc_strtok(options->env, NULL, " ");

        dc_strcpy(options->env, body, login_token);
        dc_strcat(options->env, body, "\3");
        dc_strcat(options->env, body, password);
        dc_strcat(options->env, body, "\3");

        fprintf(options->debug_log_file, "Body %s\n", body); // Write a string to the file
        clear_debug_file_buffer(options->debug_log_file);
        send_create_auth(options->env, options->err, options->socket_fd, body);
    }
}

void handle_read(struct server_options * options, struct request * request) {
    char body[MAX_SIZE];

    fprintf(options->debug_log_file, "HANDLE READ\n"); // Write a string to the file
    clear_debug_file_buffer(options->debug_log_file);

    if (dc_strcmp(options->env, "U", request->obj) == 0)
    {

    } else if (dc_strcmp(options->env, "C", request->obj) == 0)
    {

    } else if (dc_strcmp(options->env, "M", request->obj) == 0)
    {
        dc_strtok(options->env, request->data, " ");
        dc_strtok(options->env, NULL, " ");
        char * channel_name = dc_strtok(options->env, NULL, " ");
        char * num_messages = dc_strtok(options->env, NULL, " ");

        dc_strcpy(options->env, body, channel_name);
        dc_strcat(options->env, body, "\3");
        dc_strcat(options->env, body, num_messages);
        dc_strcat(options->env, body, "\3");

        fprintf(options->debug_log_file, "Body %s\n", body); // Write a string to the file
        clear_debug_file_buffer(options->debug_log_file);
        send_read_message(options->env, options->err, options->socket_fd, body);
    } else if (dc_strcmp(options->env, "A", request->obj) == 0)
    {

    }
}

void handle_update(struct server_options * options, struct request * request) {
    char body[MAX_SIZE];

    fprintf(options->debug_log_file, "HANDLE UPDATE\n"); // Write a string to the file
    clear_debug_file_buffer(options->debug_log_file);

    if (dc_strcmp(options->env, "U", request->obj) == 0)
    {

    } else if (dc_strcmp(options->env, "C", request->obj) == 0)
    {
        // JOIN or LEAVE
        dc_strtok(options->env, request->data, " ");
        dc_strtok(options->env, NULL, " ");
        char * status = dc_strtok(options->env, NULL, " ");

        char * display_name = dc_strtok(options->env, NULL, " ");
        char * channel_name = dc_strtok(options->env, NULL, " ");
        // stupid
        dc_strcpy(options->env, body, channel_name);
        dc_strcat(options->env, body, "\3");
        dc_strcat(options->env, body, "0\3");
        dc_strcat(options->env, body, "0\3");
        if (dc_strcmp(options->env, status, "J") == 0) {
            dc_strcat(options->env, body, "1\3");
        } else {
            // Leave
            dc_strcat(options->env, body, "2\3");
        }
        dc_strcat(options->env, body, "1\3");
        dc_strcat(options->env, body, display_name);
        dc_strcat(options->env, body, "\3");
        dc_strcat(options->env, body, "0\3");
        dc_strcat(options->env, body, "0\3");

        fprintf(options->debug_log_file, "Body %s\n", body); // Write a string to the file
        clear_debug_file_buffer(options->debug_log_file);

        send_update_channel(options->env, options->err, options->socket_fd, body);
    } else if (dc_strcmp(options->env, "M", request->obj) == 0)
    {

    } else if (dc_strcmp(options->env, "A", request->obj) == 0)
    {

    }
}

void handle_delete(struct server_options * options, struct request * request) {
    char body[MAX_SIZE];

    fprintf(options->debug_log_file, "HANDLE UPDATE\n"); // Write a string to the file
    clear_debug_file_buffer(options->debug_log_file);

    if (dc_strcmp(options->env, "U", request->obj) == 0)
    {

    } else if (dc_strcmp(options->env, "C", request->obj) == 0)
    {

    } else if (dc_strcmp(options->env, "M", request->obj) == 0)
    {

    } else if (dc_strcmp(options->env, "A", request->obj) == 0)
    {
        dc_strtok(options->env, request->data, " ");
        dc_strtok(options->env, NULL, " ");
        char * display_name = dc_strtok(options->env, NULL, " ");

        dc_strcpy(options->env, body, display_name);
        dc_strcat(options->env, body, "\3");

        fprintf(options->debug_log_file, "Body %s\n", body); // Write a string to the file
        clear_debug_file_buffer(options->debug_log_file);
        send_delete_auth(options->env, options->err, options->socket_fd, body);
    }
}
