#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>

#define PORT 8080
#define BUF_SIZE 8096

typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
} client_info;

bool debug_mode = false;  // Global flag for debug mode

void send_html_response(int client_socket) {
    char response[BUF_SIZE];
    FILE *html_file = fopen("/usr/local/sbin/web.html", "r");

    if (html_file == NULL) {
        perror("Error opening web.html");
        snprintf(response, sizeof(response),
                 "HTTP/1.1 500 Internal Server Error\r\n"
                 "Content-Type: text/plain\r\n"
                 "\r\n"
                 "500 Internal Server Error: Could not open web.html file.");
        send(client_socket, response, strlen(response), 0);
        return;
    }

    snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n");
    send(client_socket, response, strlen(response), 0);

    while (fgets(response, sizeof(response), html_file) != NULL) {
        send(client_socket, response, strlen(response), 0);
    }

    fclose(html_file);
}

void send_json_data(int client_socket) {
    FILE *fp;
    char line[99];

    fp = fopen("/tmp/bsec", "r");
    if (fp == NULL) {
        if (debug_mode) printf("Error opening file /tmp/bsec\n");
        return;
    }
    
    if (fgets(line, sizeof(line), fp) == NULL) {
        if (debug_mode) printf("Error reading from file /tmp/bsec\n");
        fclose(fp);
        return;
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

    sprintf(cmd, "rad.sh");
    fp_rad = popen(cmd, "r");
    if (fp_rad == NULL) {
        if (debug_mode) printf("Failed to run command\n");
        return;
    }

    while (fgets(rad_line, sizeof(rad_line), fp_rad) != NULL) {
        sscanf(rad_line, "%d %d", &rad[0], &rad[1]);
    }
    pclose(fp_rad);

    char json_response[BUF_SIZE];
    snprintf(json_response, sizeof(json_response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "\r\n"
        "{"
        "\"temp\": %.1f, \"raw_temp\": %.1f, \"humidity\": %.1f, \"raw_hum\": %.1f,"
        "\"press\": %.2f, \"gas\": %.0f, \"ecCO2\": %.0f, \"bVOC\": %.2f,"
        "\"IAQ\": %.0f, \"SIAQ\": %.0f, \"IAQ_ACC\": %.0f, \"status\": %.0f,"
        "\"dyn_rad\": %d, \"stat_rad\": %d"
        "}", arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], arr[6], arr[7],
        arr[8], arr[9], arr[10], arr[11], rad[0], rad[1]);

    send(client_socket, json_response, strlen(json_response), 0);
}

void *handle_client(void *arg) {
    client_info *client = (client_info *)arg;
    int client_socket = client->client_socket;

    char request[BUF_SIZE];
    int received = recv(client_socket, request, BUF_SIZE - 1, 0);
    if (received < 0) {
        if (debug_mode) perror("Error receiving request");
        close(client_socket);
        free(client);
        pthread_exit(NULL);
    }
    request[received] = '\0';

    if (strstr(request, "GET /data") != NULL) {
        send_json_data(client_socket);
    } else {
        send_html_response(client_socket);
    }

    close(client_socket);
    free(client);
    pthread_exit(NULL);
}

volatile sig_atomic_t server_running = 1;
int server_socket;

void handle_signal(int sig) {
    if (sig == SIGINT) {
        if (debug_mode) printf("\nReceived SIGINT. Shutting down server...\n");
        server_running = 0;
        close(server_socket);
    }
}

int main(int argc, char *argv[]) {
    // Check for debug flag
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            debug_mode = true;
            break;
        }
    }

    struct sockaddr_in server_addr;
    socklen_t size;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error establishing socket");
        exit(EXIT_FAILURE);
    }

    if (debug_mode) printf("=> Socket server has been created...\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding connection");
        exit(EXIT_FAILURE);
    }

    size = sizeof(server_addr);
    if (debug_mode) printf("=> Looking for clients...\n");

    listen(server_socket, 5);

    signal(SIGINT, handle_signal);

    pthread_t tid;
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

        if (debug_mode) printf("=> Connected with the client, you are good to go...\n");

        if (pthread_create(&tid, NULL, handle_client, (void *)client) != 0) {
            perror("Could not create thread");
            free(client);
            continue;
        }

        pthread_detach(tid);
    }

    if (debug_mode) {
        printf("Closing server socket...\n");
        printf("Goodbye...\n");
    }

    close(server_socket);

    return 0;
}
