// squdpclient.c - Connected Mode (TCP)
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

#include "sqserv.h"

char *prog_name;

void usage() {
    printf("usage: %s [-p <port>] <hostname> <integer>\n", prog_name);
    printf("\t-p <port> specify alternate port\n");
}

int main(int argc, char* argv[]) {
    long port = DEFAULT_PORT;
    char *host;
    int fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    ssize_t len;
    long num;
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

    if (argc != 2) {
        usage();
        return EX_USAGE;
    }

    host = argv[0];
    num = strtol(argv[1], NULL, 10);

    printf("Connecting to %s:%ld\n", host, port);

    // Create a TCP socket
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        err(EX_SOFTWARE, "Error creating socket");
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        close(fd);
        err(EX_NOHOST, "Invalid server IP address");
    }

    // Connect to the server
    if (connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(fd);
        err(EX_SOFTWARE, "Error connecting to server");
    }

    // Send integer to the server
    snprintf(buffer, BUFFER_SIZE, "%ld", num);
    if (send(fd, buffer, strlen(buffer), 0) < 0) {
        close(fd);
        err(EX_SOFTWARE, "Error sending data");
    }

    // Receive the squared result from the server
    len = recv(fd, buffer, BUFFER_SIZE, 0);
    if (len < 0) {
        close(fd);
        err(EX_SOFTWARE, "Error receiving data");
    }
    buffer[len] = '\0';
    printf("Squared result from server: %s\n", buffer);

    // Clean up and close the connection
    close(fd);
    return EX_OK;
}
