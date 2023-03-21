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

#define SERVER_PORT 5000
#define MAX_SIZE 1024

int main(int argc, char *argv[])
{
    struct dc_env *env;
    struct dc_error *err;
    int socket_fd;
    struct sockaddr_in server_addr;
    char buffer[MAX_SIZE];
    bool run_client = true;

    FILE *fp;
    fp = fopen("debug.txt", "w"); // Open the file for writing

    if (fp == NULL) {
        printf("Failed to create debug file\n");
        return 1;
    }

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <server_ip>\n", argv[0]);
        run_client = false;
    }

    err = dc_error_create(true);
    env = dc_env_create(err, true, NULL);

    char buffer2[1024];
    ssize_t num_read = read(STDIN_FILENO, buffer2, sizeof(buffer2));
    if (num_read == -1) {
        perror("read failed");
        exit(EXIT_FAILURE);
    }
    // Write the string to the file
    fprintf(fp, "%s\n", buffer2); // Write a string to the file

    // Clear buffer
    fclose(fp); // Close the file

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0)
    {
        perror("socket");
        run_client = false;
    }

    dc_memset(env, &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        run_client = false;
    }

    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        run_client = false;
    }

    if (run_client) {
        fprintf(stderr, "Connected to server.\n");

        while(fgets(buffer, MAX_SIZE, stdin) != NULL)
        {
            ssize_t n1 = send(socket_fd, buffer, dc_strlen(env, buffer), 0);

            if (n1 < 0)
            {
                perror("send");
                return EXIT_FAILURE;
            }

            fprintf(stderr, "Written to server\n");
            write(STDOUT_FILENO, buffer, n1);

            uint32_t header;
            char body[MAX_SIZE];
            ssize_t n;

            // receive header from server
            n = read(socket_fd, &header, sizeof(header));
            if (n < 0) {
                perror("read error");
                exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
            }

            struct binary_header_field * binaryHeaderField = deserialize_header(header);

            // print deserialized header
            fprintf(stderr, "Version: %d\n", binaryHeaderField->version);
            fprintf(stderr, "Type: %d\n", binaryHeaderField->type);
            fprintf(stderr, "Object: %d\n", binaryHeaderField->object);
            fprintf(stderr, "Body Size: %d\n", binaryHeaderField->body_size);

            // Read body and clear buffer
            read(socket_fd, &body, MAX_SIZE);
            body[(binaryHeaderField->body_size)] = '\0';
            fprintf(stderr, "Body: %s\n", body);
        }
//    while (true) {
//        // Check semaphore and if data then read, parse and send to server
//        uint32_t header;
//        ssize_t bytes_received = recv(socket_fd, &header, sizeof(header), 0);
//        if (bytes_received == 0) {
//            // Connection closed by remote host
//            break;
//        } else if (bytes_received < 0) { // NOLINT(llvm-else-after-return,readability-else-after-return)
//            // Error receiving data
//            perror("recv");
//            exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
//        } else {
//            // Process received data
//            char body[MAX_SIZE];
//
//            struct binary_header_field * binaryHeaderField = deserialize_header(header);
//
//            // print deserialized header
//            fprintf(stderr, "Version: %d\n", binaryHeaderField->version);
//            fprintf(stderr, "Type: %d\n", binaryHeaderField->type);
//            fprintf(stderr, "Object: %d\n", binaryHeaderField->object);
//            fprintf(stderr, "Body Size: %d\n", binaryHeaderField->body_size);
//            // Read body and clear buffer
//            read(socket_fd, &body, MAX_SIZE);
//            body[(binaryHeaderField->body_size)] = '\0';
//            fprintf(stderr, "Body: %s\n", body);
//        }
//    }
        fprintf(stderr, "Client Disconnected.\n");
    }

    free(env);
    free(err);
    close(socket_fd);

    return EXIT_SUCCESS;
}

//#include "../include/protocol_util.h"
//#include <arpa/inet.h>
//#include <dc_c/dc_stdio.h>
//#include <dc_c/dc_stdlib.h>
//#include <dc_c/dc_string.h>
//#include <dc_env/env.h>
//#include <dc_error/error.h>
//#include <dc_posix/dc_unistd.h>
//#include <dc_util/io.h>
//#include <dlfcn.h>
//#include <netinet/in.h>
//#include <stdbool.h>
//#include <sys/socket.h>
//#include <pthread.h>
//
//#define SERVER_PORT 5000
//#define MAX_SIZE 1024
//
//struct server_options{
//    struct dc_env *env;
//    struct dc_error *err;
//    int socket_fd;
//};
//
//void * handle_ui(void * arg);
//void * handle_server(void * arg);
//
//int main(int argc, char *argv[])
//{
//    struct dc_env *env;
//    struct dc_error *err;
//    int socket_fd;
//    struct sockaddr_in server_addr;
//    bool run_client = true;
//
//    if (argc < 2)
//    {
//        fprintf(stderr, "Usage: %s <server_ip>\n", argv[0]);
//        run_client = false;
//    }
//
//    err = dc_error_create(true);
//    env = dc_env_create(err, true, NULL);
//
//
//    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
//
//    if (socket_fd < 0)
//    {
//        perror("socket");
//        run_client = false;
//    }
//
//    dc_memset(env, &server_addr, 0, sizeof(server_addr));
//    server_addr.sin_family = AF_INET;
//    server_addr.sin_port = htons(SERVER_PORT);
//
//    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
//    {
//        perror("inet_pton");
//        run_client = false;
//    }
//
//    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
//    {
//        perror("connect");
//        run_client = false;
//    }
//
//    if (run_client) {
//        struct server_options options = {env, err, socket_fd};
//        // Create two threads, one for reading from stdin and one for reading from socket
//        pthread_t read_thread, write_thread;
//
//        pthread_create(&read_thread, NULL, handle_ui, NULL);
//        pthread_create(&write_thread, NULL, handle_server, &options);
//
//        pthread_join(read_thread, NULL);
//        pthread_join(write_thread, NULL);
//    }
//
//    free(env);
//    free(err);
//    close(socket_fd);
//
//    return EXIT_SUCCESS;
//}
//
//void * handle_ui(void * arg)
//{
//    char buffer2[MAX_SIZE];
//
//    ssize_t num_read = read(STDIN_FILENO, buffer2, MAX_SIZE);
//    if (num_read == -1) {
//        perror("read failed");
//        return NULL;
//    }
//
//    fprintf(stderr, "Child process received: %.*s", (int)num_read, buffer2);
//
//    return NULL;
//}
//
//void * handle_server(void * arg)
//{
//    struct server_options * options = (struct server_options *) arg;
//    char buffer[MAX_SIZE];
//
//    fprintf(stderr, "Connected to server.\n");
//    while(fgets(buffer, MAX_SIZE, stdin) != NULL)
//    {
//        ssize_t n1 = send(options->socket_fd, buffer, dc_strlen(options->env, buffer), 0);
//
//        if (n1 < 0)
//        {
//            perror("send");
//            return NULL;
//        }
//
//        fprintf(stderr, "Written to server\n");
//        write(STDOUT_FILENO, buffer, n1);
//
//        uint32_t header;
//        char body[MAX_SIZE];
//        ssize_t n;
//
//        // receive header from server
//        n = read(options->socket_fd, &header, sizeof(header));
//        if (n < 0) {
//            perror("read error");
//            exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
//        }
//
//        struct binary_header_field * binaryHeaderField = deserialize_header(header);
//
//        // print deserialized header
//        fprintf(stderr, "Version: %d\n", binaryHeaderField->version);
//        fprintf(stderr, "Type: %d\n", binaryHeaderField->type);
//        fprintf(stderr, "Object: %d\n", binaryHeaderField->object);
//        fprintf(stderr, "Body Size: %d\n", binaryHeaderField->body_size);
//
//        // Read body and clear buffer
//        read(options->socket_fd, &body, MAX_SIZE);
//        body[(binaryHeaderField->body_size)] = '\0';
//        fprintf(stderr, "Body: %s\n", body);
//    }
//
//    fprintf(stderr, "Client Disconnected.\n");
//    return NULL;
//}
