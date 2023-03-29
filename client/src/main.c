#include "protocol_util.h"
#include "handle_ui.h"
#include "handle_server.h"
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
#include <dc_util/types.h>

#define SERVER_PORT 5000

void * handle_ui(void * arg);
void * handle_server(void * arg);

int main(int argc, char *argv[])
{
    FILE * debug_log_file;
    struct dc_env *env;
    struct dc_error *err;
    int socket_fd;
    int port = SERVER_PORT;
    struct sockaddr_in server_addr;
    bool run_client = true;

    err = dc_error_create(true);
    env = dc_env_create(err, true, NULL);

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <server_ip>\n", argv[0]);
        run_client = false;
    } else if (argc == 3) {
        port = dc_uint16_from_str(env, err, argv[2], 10);
    }

    // Open the debugging file
    debug_log_file = fopen("debug.txt", "w");

    if (debug_log_file == NULL) {
        printf("Failed to create debug file\n");
        run_client = false;
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0)
    {
        run_client = false;
    }

    dc_memset(env, &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
    {
        run_client = false;
    }

    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        run_client = false;
    }

    if (run_client) {
        write(STDOUT_FILENO, "Server Running\n", dc_strlen(env, "Server Running\n"));

        struct server_options options = {env, err, debug_log_file, socket_fd};

//        char * request = dc_strdup(env, err, "CREATE U a@gmail wumbo pass1234");
//        char * request2 = dc_strdup(env, err, "CREATE A wumbo pass1234");
//
//        struct request *  request_obj = get_type_and_object(&options, request);
//        handle_ui_request(&options, request_obj);
//        free(request_obj->type);
//        free(request_obj->obj);
//        free(request_obj->data);
//        free(request2);
//        free(request);
//        free(request_obj);
//      Create two threads, one for reading from stdin and one for reading from socket
        pthread_t ui_thread, server_thread;

        pthread_create(&ui_thread, NULL, handle_ui, &options);
        pthread_create(&server_thread, NULL, handle_server, &options);

        pthread_join(ui_thread, NULL);
        pthread_join(server_thread, NULL);
    } else {
        write(STDOUT_FILENO, "Server Failed to Connect\n", dc_strlen(env, "Server Failed to Connect\n"));
    }

    free(env);
    dc_error_reset(err);
    free(err);
    close(socket_fd);
    fclose(debug_log_file);
    return EXIT_SUCCESS;
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
        request[num_read] = '\0';
        // Write the string to the file
        fprintf(options->debug_log_file, "%s\n", request); // Write a string to the file

        struct request *  request_obj = get_type_and_object(options, dc_strdup(options->env, options->err, request));
        handle_ui_request(options, request_obj);

        // Clear buffer
        clear_debug_file_buffer(options->debug_log_file);
        free(request_obj->type);
        free(request_obj->obj);
        free(request_obj->data);
        free(request_obj);
    }
}

void * handle_server(void * arg)
{
    struct server_options * options = (struct server_options *) arg;
    fprintf(options->debug_log_file, "Connected to server.\n"); // Write a string to the file
    clear_debug_file_buffer(options->debug_log_file);

    while (true) {
        uint32_t header;
        char body[MAX_SIZE];
        ssize_t n;

        // receive header from server
        n = dc_read_fully(options->env, options->err, options->socket_fd, &header, sizeof(header));

        if (n < 0) {
            fprintf(options->debug_log_file, "Failed to read header from server.\n"); // Write a string to the file
            clear_debug_file_buffer(options->debug_log_file);
            exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
        } else if (n == 0) {
            fprintf(options->debug_log_file, "Server closed connection.\n"); // Write a string to the file
            clear_debug_file_buffer(options->debug_log_file);
        }

        struct binary_header_field * binaryHeaderField = deserialize_header(header);

        // print deserialized header
        fprintf(options->debug_log_file, "HEADER CONTENTS.\n"); // Write a string to the file
        fprintf(options->debug_log_file, "Version: %d\n", binaryHeaderField->version);
        fprintf(options->debug_log_file, "Type: %d\n", binaryHeaderField->type);
        fprintf(options->debug_log_file, "Object: %d\n", binaryHeaderField->object);
        fprintf(options->debug_log_file, "Body Size: %d\n", binaryHeaderField->body_size);
        clear_debug_file_buffer(options->debug_log_file);

        // Read body and clear buffer
        read(options->socket_fd, &body, MAX_SIZE);
        body[(binaryHeaderField->body_size)] = '\0';
        fprintf(options->debug_log_file, "Body: %s\n", body);

        // Handle server response
        handle_server_request(options, binaryHeaderField, body);

        clear_debug_file_buffer(options->debug_log_file);
    }
}
