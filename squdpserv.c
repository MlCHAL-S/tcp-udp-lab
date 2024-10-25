// squdpserv.c - Connected Mode with select() for Multiple Clients
#include <errno.h>
#include <err.h>
#include <sysexits.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <sys/select.h>

#include "sqserv.h"

char *prog_name;

void usage() {
    printf("usage: %s [-p <port>]\n", prog_name);
    printf("\t-p <port> specify alternate port\n");
}

int main(int argc, char* argv[]) {
    long port = DEFAULT_PORT;
    struct sockaddr_in server_addr, client_addr;
    int server_fd, client_fd, max_fd;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    ssize_t len;
    long received_num, result;
    fd_set active_fd_set, read_fd_set;
    char *end;
    int ch;

    prog_name = strdup(basename(argv[0]));
    while ((ch = getopt(argc, argv, "?hp:")) != -1) {
        switch (ch) {
            case 'p': port = strtol(optarg, NULL, 10); break;
            case 'h':
            case '?':
            default: usage(); return 0;
        }
    }
    argc -= optind;
    argv += optind;

    if (argc != 0) {
        usage();
        return EX_USAGE;
    }

    printf("Server is listening on port %ld\n", port);

    // Create a TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        err(EX_SOFTWARE, "Error creating socket");
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind the socket to the specified port and IP
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(server_fd);
        err(EX_SOFTWARE, "Error binding socket");
    }

    // Start listening for incoming connections
    if (listen(server_fd, 5) < 0) {
        close(server_fd);
        err(EX_SOFTWARE, "Error listening on socket");
    }

    // Initialize the set of active sockets
    FD_ZERO(&active_fd_set);
    FD_SET(server_fd, &active_fd_set);
    max_fd = server_fd;

    while (1) {
        // Create a copy of the active_fd_set to pass to select
        read_fd_set = active_fd_set;

        // Wait for activity on any of the sockets
        if (select(max_fd + 1, &read_fd_set, NULL, NULL, NULL) < 0) {
            perror("select failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // Iterate through all possible file descriptors
        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fd_set)) {
                // Handle new connection on the server socket
                if (i == server_fd) {
                    client_len = sizeof(client_addr);
                    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                    if (client_fd < 0) {
                        perror("accept failed");
                        continue;
                    }

                    printf("New client connected from IP %s, port %d\n",
                           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    // Add the new client socket to the active set
                    FD_SET(client_fd, &active_fd_set);
                    if (client_fd > max_fd) {
                        max_fd = client_fd;
                    }
                } 
                // Handle data from an existing client
                else {
                    len = recv(i, buffer, BUFFER_SIZE, 0);
                    if (len <= 0) {
                        if (len < 0) {
                            perror("recv failed");
                        } else {
                            printf("Client on socket %d disconnected\n", i);
                        }
                        close(i);
                        FD_CLR(i, &active_fd_set);
                    } else {
                        buffer[len] = '\0';

                        // Convert the received string to a number
                        received_num = strtol(buffer, &end, 10);
                        if (errno == ERANGE || (errno == EINVAL && received_num == 0 && end == buffer)) {
                            fprintf(stderr, "Received invalid number from client\n");
                        } else {
                            // Compute the square and send back the result
                            result = received_num * received_num;
                            printf("Received: %ld, Squared: %ld\n", received_num, result);

                            snprintf(buffer, BUFFER_SIZE, "%ld", result);
                            if (send(i, buffer, strlen(buffer), 0) < 0) {
                                perror("send failed");
                            }
                        }
                    }
                }
            }
        }
    }

    close(server_fd);
    return EX_OK;
}
