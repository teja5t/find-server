#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define IP_ADDR "127.0.0.1"
#define BUFLEN  128

int create_socket(struct timeval *tv) {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        fprintf(stderr, "Error: Failed to create socket. %s.\n", strerror(errno));
        return -1;
    }

    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, tv, sizeof(*tv)) == -1) {
        fprintf(stderr, "Error: Cannot set socket options. %s.\n", strerror(errno));
        close(client_socket);
        return -1;
    }

    return client_socket;
}

int main() {
    int client_socket = -1, bytes_recvd, retval = EXIT_SUCCESS;
    struct sockaddr_in serv_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    char buf[BUFLEN];

    memset(&serv_addr, 0, addrlen);
    int ip_conversion = inet_pton(AF_INET, IP_ADDR, &serv_addr.sin_addr);
    if (ip_conversion == 0) {
        fprintf(stderr, "Error: Invalid IP address '%s'.\n", IP_ADDR);
        return -1;
    } else if (ip_conversion < 0) {
        fprintf(stderr, "Error: Failed to convert IP address. %s.\n", strerror(errno));
        return -1;
    }
    serv_addr.sin_family = AF_INET;

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 250; // 250 microseconds

    for (int i = 1024; i <= 65535; i++) {
        serv_addr.sin_port = htons(i);

        client_socket = create_socket(&tv);
        if (client_socket == -1) {
            retval = EXIT_FAILURE;
            goto EXIT;
        }

        if (connect(client_socket, (struct sockaddr *)&serv_addr, addrlen) != -1) {
            bytes_recvd = recv(client_socket, buf, BUFLEN - 1, 0);
            if (bytes_recvd > 0) {
                buf[bytes_recvd] = '\0';
                printf("Found server on port %d.\n", i);
                printf("Received message from server: %s\n", buf);
                goto EXIT;
            }
        }

        close(client_socket);
        client_socket = -1;
    }

    printf("No server was found.\n");

EXIT:
    if (client_socket != -1 && fcntl(client_socket, F_GETFD) != -1) {
        close(client_socket);
    }
    return retval;
}
