// squdpserv.c
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

#include "sqserv.h"  // Definitions for DEFAULT_PORT and BUFFER_SIZE

char *prog_name;

void usage() {
    printf("usage: %s [-p <port>]\n", prog_name);
    printf("\t-p <port> specify alternate port\n");
}

int main(int argc, char* argv[]) {
    long port = DEFAULT_PORT;
    struct sockaddr_in sin, client_addr;
    int fd;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    ssize_t len;
    long received_num, result;
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

    // Create a UDP socket
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        err(EX_SOFTWARE, "Error creating socket");
    }

    // Initialize server address structure
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    // Bind the socket to the specified port and IP
    if (bind(fd, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        close(fd);
        err(EX_SOFTWARE, "Error binding socket");
    }

    // Receive data from the client
    client_len = sizeof(client_addr);
    len = recvfrom(fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
    if (len < 0) {
        close(fd);
        err(EX_SOFTWARE, "Error receiving data");
    }
    buffer[len] = '\0';

    received_num = strtol(buffer, &end, 10);
    if (errno == ERANGE || (errno == EINVAL && received_num == 0 && end == buffer)) {
        err(EX_DATAERR, "Received invalid number");
    }

    result = received_num * received_num;
    printf("Received: %ld, Squared: %ld\n", received_num, result);

    // Send squared result back to client
    snprintf(buffer, BUFFER_SIZE, "%ld", result);
    if (sendto(fd, buffer, strlen(buffer), 0, (struct sockaddr*)&client_addr, client_len) < 0) {
        close(fd);
        err(EX_SOFTWARE, "Error sending data");
    }

    close(fd);
    return EX_OK;
}
