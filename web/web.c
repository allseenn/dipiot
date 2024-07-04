#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>

#define PORT 8000
#define BUF_SIZE 1024

typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
} client_info;

void *handle_client(void *arg) {
    client_info *client = (client_info *)arg;
    char buffer[BUF_SIZE];
    int client_socket = client->client_socket;

    while (1) {
        int result = recv(client_socket, buffer, BUF_SIZE, 0);
        if (result < 0) {
            printf("\n\n=> Connection terminated error %d with IP %s\n", result, inet_ntoa(client->client_addr.sin_addr));
            break;
        } else if (result == 0) {
            printf("\n\n=> Connection closed by client IP %s\n", inet_ntoa(client->client_addr.sin_addr));
            break;
        }

        buffer[result] = '\0';
        printf("%s\n", buffer);

        if (strstr(buffer, "GET /on1") != NULL) {
            system("echo 'LED #1 is ON'");
        } else if (strstr(buffer, "GET /off1") != NULL) {
            system("echo 'LED #1 is OFF'");
        } else if (strstr(buffer, "GET /on2") != NULL) {
            system("echo 'LED #2 is ON'");
        } else if (strstr(buffer, "GET /off2") != NULL) {
            system("echo 'LED #2 is OFF'");
        }

        char response[1024] = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: text/html; charset=utf-8\r\n"
                              "\r\n"
                              "<!DOCTYPE HTML>"
                              "<html>"
                              "  <head>"
                              "    <meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
                              "  </head>"
                              "  <h1>ODROID - Web Server</h1>"
                              "  <p>LED #1"
                              "    <a href=\"on1\">"
                              "      <button>ON</button>"
                              "    </a>&nbsp;"
                              "    <a href=\"off1\">"
                              "      <button>OFF</button>"
                              "    </a>"
                              "  </p>"
                              "  <p>LED #2"
                              "    <a href=\"on2\">"
                              "      <button>ON</button>"
                              "    </a>&nbsp;"
                              "    <a href=\"off2\">"
                              "      <button>OFF</button>"
                              "    </a>"
                              "  </p>"
                              "</html>";

        send(client_socket, response, strlen(response), 0);
    }

    close(client_socket);
    free(client);
    pthread_exit(NULL);
}

volatile sig_atomic_t server_running = 1;

void handle_signal(int sig) {
    if (sig == SIGINT) {
        printf("\nReceived SIGINT. Shutting down server...\n");
        server_running = 0;
    }
}

int main() {
    int server;
    struct sockaddr_in server_addr;
    socklen_t size;

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        perror("Error establishing socket");
        exit(EXIT_FAILURE);
    }

    printf("=> Socket server has been created...\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding connection");
        exit(EXIT_FAILURE);
    }

    size = sizeof(server_addr);
    printf("=> Looking for clients...\n");

    listen(server, 5);

    signal(SIGINT, handle_signal);

    pthread_t tid;
    while (server_running) {
        client_info *client = malloc(sizeof(client_info));
        client->client_socket = accept(server, (struct sockaddr*)&client->client_addr, &size);

        if (client->client_socket < 0) {
            perror("Error on accepting");
            free(client);
            continue;
        }

        printf("=> Connected with the client, you are good to go...\n");

        if (pthread_create(&tid, NULL, handle_client, (void *)client) != 0) {
            perror("Could not create thread");
            free(client);
            continue;
        }

        pthread_detach(tid);
    }

    printf("Closing server socket...\n");
    close(server);
    printf("Goodbye...\n");

    return 0;
}
