#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define SERVER_PORT 5000
#define BUF_SIZE 256
#define MAX_SIZE 1024


enum Type {
    CREATE = 0x1,
    READ = 0x2,
    UPDATE = 0x3,
    DESTROY = 0x4,
    PING = 0x8
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
struct binary_header_field * deserialize(uint32_t value);

int main(int argc, char *argv[])
{
    int socket_fd;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];

    if (argc < 2)
    {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return EXIT_FAILURE;
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        return EXIT_FAILURE;
    }

    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        return EXIT_FAILURE;
    }

    printf("Connected to server.\n");

    while(fgets(buffer, BUF_SIZE, stdin) != NULL)
    {
        ssize_t n1 = send(socket_fd, buffer, strlen(buffer), 0);

        if (n1 < 0)
        {
            perror("send");
            return EXIT_FAILURE;
        }

        printf("Written to server\n");
        write(STDOUT_FILENO, buffer, n1);

        uint32_t header;
        char body[MAX_SIZE];
        ssize_t n;

        // receive header from server
        n = read(socket_fd, &header, sizeof(header));
        if (n < 0) {
            perror("read error");
            exit(EXIT_FAILURE);
        }

        header = ntohl(header);
        struct binary_header_field * binaryHeaderField = deserialize(header);

        // print deserialized header
        printf("Version: %d\n", binaryHeaderField->version);
        printf("Type: %d\n", binaryHeaderField->type);
        printf("Object: %d\n", binaryHeaderField->object);
        printf("Body Size: %d\n", binaryHeaderField->body_size);
        printf("N: %zd\n", n);
        read(socket_fd, &body, MAX_SIZE);
        body[(binaryHeaderField->body_size+1)] = '\0';
        printf("Body: %s\n", body);
    }

    close(socket_fd);

    return EXIT_SUCCESS;
}

struct binary_header_field * deserialize(uint32_t value) {
    struct binary_header_field * header;
    header->version = (value >> 28) & 0x0F;
    header->type = (value >> 24) & 0x0F;
    header->object = (value >> 16) & 0xFF;
    header->body_size = value & 0xFFFF;
    return header;
}
