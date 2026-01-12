#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 4096

void send_response(int client_fd,
                   const char *status,
                   const char *content_type,
                   const char *body,
                   size_t body_len) {

    char header[512];
    snprintf(header, sizeof(header),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, content_type, body_len
    );

    send(client_fd, header, strlen(header), 0);
    send(client_fd, body, body_len, 0);
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    setbuf(stdout, NULL);
    signal(SIGCHLD, SIG_IGN);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(1);
    }

    printf("Server running on http://localhost:%d\n", PORT);

    while (1) {
        int client_fd = accept(server_fd,
                               (struct sockaddr *)&address,
                               &addrlen);
        if (client_fd < 0) continue;

        pid_t pid = fork();

        if (pid == 0) {
            /* CHILD */
            close(server_fd);

            char buffer[BUFFER_SIZE];
            memset(buffer, 0, BUFFER_SIZE);
            read(client_fd, buffer, BUFFER_SIZE);

            char method[8], path[256];
            sscanf(buffer, "%s %s", method, path);

            printf("PID %d handling %s %s\n", getpid(), method, path);

            const char *file_path = NULL;

            if (strcmp(path, "/") == 0) {
                file_path = "public/index.html";
            }

            if (file_path == NULL) {
                const char *msg = "404 Not Found";
                send_response(client_fd,
                              "404 Not Found",
                              "text/plain",
                              msg,
                              strlen(msg));
                close(client_fd);
                exit(0);
            }

            int file_fd = open(file_path, O_RDONLY);
            if (file_fd < 0) {
                const char *msg = "404 Not Found";
                send_response(client_fd,
                              "404 Not Found",
                              "text/plain",
                              msg,
                              strlen(msg));
                close(client_fd);
                exit(0);
            }

            struct stat st;
            fstat(file_fd, &st);

            char *file_data = malloc(st.st_size);
            read(file_fd, file_data, st.st_size);

            send_response(client_fd,
                          "200 OK",
                          "text/html",
                          file_data,
                          st.st_size);

            free(file_data);
            close(file_fd);
            close(client_fd);
            exit(0);
        }

        /* PARENT */
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
