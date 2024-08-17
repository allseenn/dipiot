#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include <stddef.h>

#define PORT 8080
#define BUF_SIZE 1024
#define MAX_RETRIES 3

void *handle_client(void *arg);
void handle_signal(int sig);

typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
} client_info;

volatile sig_atomic_t server_running = 1;
int server_socket;

void *handle_client(void *arg) {
    client_info *client = (client_info *)arg;
    int client_socket = client->client_socket;
    int retries = 0;

    FILE *fp;
    char line[99];

    fp = fopen("/tmp/bsec", "r");
    if (fp == NULL) {
        printf("Error opening file /tmp/bsec\n");
        goto cleanup;
    }

    if (fgets(line, sizeof(line), fp) == NULL) {
        printf("Error reading from file /tmp/bsec\n");
        fclose(fp);
        goto cleanup;
    }

    fclose(fp);

    float arr[12] = {0};
    int arr_i = 0;
    char *token = strtok(line, " ");
    while (token != NULL) {
        arr[arr_i++] = atof(token);
        token = strtok(NULL, " ");
    }

    int rad[2];
    char cmd[100];
    FILE *fp_rad;
    char rad_line[50];

    sprintf(cmd, "./rad.sh");
    fp_rad = popen(cmd, "r");
    if (fp_rad == NULL) {
        printf("Failed to run command\n");
        goto cleanup;
    }

    while (fgets(rad_line, sizeof(rad_line), fp_rad) != NULL) {
        sscanf(rad_line, "%d %d", &rad[0], &rad[1]);
    }
    pclose(fp_rad);

    char response[BUF_SIZE];
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<!DOCTYPE HTML>"
        "<!-- ... (остальной HTML-код) ... -->");

    while (retries < MAX_RETRIES) {
        ssize_t bytes_sent = write(client_socket, response, strlen(response));
        if (bytes_sent < 0) {
            perror("Error writing to client socket");
            retries++;
            continue;
        }
        break;
    }

    if (retries == MAX_RETRIES) {
        printf("Failed to send response to client after %d retries\n", MAX_RETRIES);
    }

cleanup:
    close(client_socket);
    free(client);
    return NULL;
}

int main() {
    struct sockaddr_in server_addr;
    socklen_t size;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error establishing socket");
        exit(EXIT_FAILURE);
    }

    printf("=> Socket server has been created...\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding connection");
        exit(EXIT_FAILURE);
    }

    size = sizeof(server_addr);
    printf("=> Looking for clients...\n");

    if (listen(server_socket, 5) < 0) {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handle_signal);

    while (server_running) {
        client_info *client = malloc(sizeof(client_info));
        client->client_socket = accept(server_socket, (struct sockaddr*)&client->client_addr, &size);

        if (client->client_socket < 0) {
            if (!server_running) {
                free(client);
                break;
            }
            perror("Error on accepting");
            free(client);
            continue;
        }

        printf("=> Connected with the client, you are good to go...\n");

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, (void *)client) != 0) {
            perror("Could not create thread");
            free(client);
            continue;
        }

        pthread_detach(tid);
    }

    printf("Closing server socket...\n");
    close(server_socket);
    printf("Goodbye...\n");

    return 0;
}

void handle_signal(int sig) {
    if (sig == SIGINT) {
        printf("Received SIGINT, shutting down server...\n");
        server_running = 0;
        close(server_socket);
    }
}
 
