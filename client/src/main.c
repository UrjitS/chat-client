#include "../include/protocol_util.h"
#include <arpa/inet.h>
#include <dc_c/dc_stdio.h>
#include <dc_c/dc_stdlib.h>
#include <dc_c/dc_string.h>
#include <dc_env/env.h>
#include <dc_error/error.h>
#include <dc_posix/dc_unistd.h>
#include <dc_util/io.h>
#include <dlfcn.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <pthread.h>
#include <dc_posix/dc_string.h>
#include <dc_util/system.h>

#define SERVER_PORT 9999
#define MAX_SIZE 1024

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
};

void clear_debug_file_buffer(FILE * debug_log_file);
struct request * get_type_and_object(struct server_options * options, char * request);
void handle_request(struct server_options * options, struct request * request);
void handle_create(struct server_options * options, struct request * request);
void handle_read(struct server_options * options, struct request * request);
void handle_update(struct server_options * options, struct request * request);
void handle_delete(struct server_options * options, struct request * request);
void * handle_ui(void * arg);
void * handle_server(void * arg);

int main(int argc, char *argv[])
{
    // TODO Fix hard code port
    FILE * debug_log_file;
    struct dc_env *env;
    struct dc_error *err;
    int socket_fd;
    struct sockaddr_in server_addr;
    bool run_client = true;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <server_ip>\n", argv[0]);
        run_client = false;
    }

    // Open the debugging file
    debug_log_file = fopen("debug.txt", "w");

    if (debug_log_file == NULL) {
        printf("Failed to create debug file\n");
        run_client = false;
    }

    err = dc_error_create(true);
    env = dc_env_create(err, true, NULL);


    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0)
    {
        run_client = false;
    }

    dc_memset(env, &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
    {
        run_client = false;
    }

    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        run_client = false;
    }

    if (run_client) {
        struct server_options options = {env, err, debug_log_file, socket_fd};

        char * request = dc_strdup(env, err, "CREATE U a@gmail wumbo 1234");
        char * request2 = dc_strdup(env, err, "CREATE A wumbo 1234");

        struct request *  request_obj = get_type_and_object(&options, request);
        handle_request(&options, request_obj);
        free(request_obj->type);
        free(request_obj->obj);
        free(request_obj->data);
        free(request2);
//        free(request);
        free(request_obj);
        // Create two threads, one for reading from stdin and one for reading from socket
//        pthread_t read_thread, write_thread;
//
//        pthread_create(&read_thread, NULL, handle_ui, &options);
//        pthread_create(&write_thread, NULL, handle_server, &options);
//
//        pthread_join(read_thread, NULL);
//        pthread_join(write_thread, NULL);
    }

    free(env);
    free(err);
    close(socket_fd);
    fclose(debug_log_file);
    return EXIT_SUCCESS;
}

void clear_debug_file_buffer(FILE * debug_log_file)
{
    fflush(debug_log_file);
    setbuf(debug_log_file, NULL);
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
    if (dc_strcmp(options->env, "U", request->obj) == 0) {
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
    } else if (dc_strcmp(options->env, "C", request->obj) == 0) {
        send_create_channel(options->env, options->err, options->socket_fd,request->data);
    } else if (dc_strcmp(options->env, "M", request->obj) == 0) {
        send_create_message(options->env, options->err, options->socket_fd,request->data);
    } else if (dc_strcmp(options->env, "A", request->obj) == 0) {
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

}

void handle_update(struct server_options * options, struct request * request) {

}

void handle_delete(struct server_options * options, struct request * request) {

}
void handle_request(struct server_options * options, struct request * request) {
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

void * handle_ui(void * arg)
{
    while (true) {
        struct server_options * options = (struct server_options *) arg;

        char request[MAX_SIZE];

        ssize_t num_read = read(STDIN_FILENO, request, sizeof(request));
        if (num_read == -1) {
            exit(EXIT_FAILURE);
        }

        // Write the string to the file
        fprintf(options->debug_log_file, "%s\n", request); // Write a string to the file

        struct request *  request_obj = get_type_and_object(options, dc_strdup(options->env, options->err, request));
        handle_request(options, request_obj);

        // Clear buffer
        clear_debug_file_buffer(options->debug_log_file);
        free(request_obj->type);
        free(request_obj->obj);
        free(request_obj->data);
        free(request_obj);
    }
    return NULL;
}

void * handle_server(void * arg)
{
    struct server_options * options = (struct server_options *) arg;
    char buffer[MAX_SIZE];

    fprintf(options->debug_log_file, "Connected to server.\n"); // Write a string to the file
    clear_debug_file_buffer(options->debug_log_file);

    while(fgets(buffer, MAX_SIZE, stdin) != NULL)
    {
        ssize_t n1 = send(options->socket_fd, buffer, dc_strlen(options->env, buffer), 0);

        if (n1 < 0)
        {
            return NULL;
        }

        fprintf(stderr, "Written to server\n");
        write(STDOUT_FILENO, buffer, n1);

        uint32_t header;
        char body[MAX_SIZE];
        ssize_t n;

        // receive header from server
        n = read(options->socket_fd, &header, sizeof(header));
        if (n < 0) {
            exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
        }

        struct binary_header_field * binaryHeaderField = deserialize_header(header);

        // print deserialized header
        fprintf(stderr, "Version: %d\n", binaryHeaderField->version);
        fprintf(stderr, "Type: %d\n", binaryHeaderField->type);
        fprintf(stderr, "Object: %d\n", binaryHeaderField->object);
        fprintf(stderr, "Body Size: %d\n", binaryHeaderField->body_size);

        // Read body and clear buffer
        read(options->socket_fd, &body, MAX_SIZE);
        body[(binaryHeaderField->body_size)] = '\0';
        fprintf(stderr, "Body: %s\n", body);
    }

    fprintf(stderr, "Client Disconnected.\n");
    return NULL;
}
