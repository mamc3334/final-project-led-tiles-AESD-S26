// Claude AI Link: https://claude.ai/chat/64445247-0e1c-4780-9e08-5e75fa7987a8

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>

#define PORT        9000
#define BACKLOG     10
#define BUFFER_SIZE 4096
#define FILE_PATH   "SendBeatmap.csv"   // <-- change this to your file

static volatile int running = 1;

static void handle_signal(int sig) {
    (void)sig;
    running = 0;
}

// Send the fixed file to the client; return 0 on success, -1 on error
static int send_file(int client_fd) {
    int file_fd = open(FILE_PATH, O_RDONLY);
    if (file_fd < 0) {
        perror("open: " FILE_PATH);
        return -1;
    }

    char buf[BUFFER_SIZE];
    ssize_t n;
    while ((n = read(file_fd, buf, sizeof(buf))) > 0) {
        char *ptr = buf;
        while (n > 0) {
            ssize_t sent = send(client_fd, ptr, n, 0);
            if (sent < 0) {
                perror("send");
                close(file_fd);
                return -1;
            }
            ptr += sent;
            n   -= sent;
        }
    }

    if (n < 0)
        perror("read");

    close(file_fd);
    return (n < 0) ? -1 : 0;
}

// Handle one client: send file on each request, stay open for more
static void handle_client(int client_fd, struct sockaddr_in *client_addr) {
    printf("Client connected: %s:%d\n",
           inet_ntoa(client_addr->sin_addr),
           ntohs(client_addr->sin_port));

    char buf[BUFFER_SIZE];
    ssize_t n;

    while ((n = recv(client_fd, buf, sizeof(buf), 0)) > 0) {
        // Any data from the client triggers a file send
        printf("Request received (%zd bytes) — sending " FILE_PATH "\n", n);

        if (send_file(client_fd) < 0)
            break;

        // Send a delimiter so the client knows the file is done
        const char *delim = "\n---EOF---\n";
        send(client_fd, delim, strlen(delim), 0);
    }

    if (n < 0 && errno != EINTR)
        perror("recv");

    printf("Client disconnected: %s:%d\n",
           inet_ntoa(client_addr->sin_addr),
           ntohs(client_addr->sin_port));

    close(client_fd);
}

int main(void) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int opt = 1;

    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;  // no SA_RESTART — let SIGINT interrupt accept()
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return EXIT_FAILURE; }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind"); close(server_fd); return EXIT_FAILURE;
    }
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen"); close(server_fd); return EXIT_FAILURE;
    }

    printf("Listening on port %d — will serve: " FILE_PATH "\n", PORT);

    while (running) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR) break;  // signal received — exit cleanly
            perror("accept");
            break;
        }
        handle_client(client_fd, &client_addr);
    }

    printf("\nShutting down.\n");
    close(server_fd);
    return EXIT_SUCCESS;
}