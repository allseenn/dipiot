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

const char *USERNAME = "admin";
const char *PASSWORD = "YWRtaW46c3R1ZGVudHM=";

typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
} client_info;

bool check_auth(const char *auth_header) {
    char expected_auth[256];
    snprintf(expected_auth, sizeof(expected_auth), "Authorization: Basic %s", PASSWORD);
    if (strcmp(auth_header, expected_auth) == 0) {
        return true;
    }
    return false;
}

void execute_command(const char *command, char *result, size_t result_size) {
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }
    fgets(result, result_size, fp);
    pclose(fp);
}

void *handle_client(void *arg) {
    client_info *client = (client_info *)arg;
    char buffer[BUF_SIZE];
    int client_socket = client->client_socket;
    
    char temp_result[256] = "";
    char pressure_result[256] = "";
    char humidity_result[256] = "";
    char gas_result[256] = "";

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
        char *auth_header = strstr(buffer, "Authorization: Basic ");
        if (!auth_header || !check_auth(auth_header)) {  
            char response[1024] = "HTTP/1.1 401 Unauthorized\r\n"
                                  "WWW-Authenticate: Basic realm=\"User Visible Realm\"\r\n"
                                  "\r\n";
            send(client_socket, response, strlen(response), 0);
            break;
        }

        if (strstr(buffer, "GET /temperature") != NULL) {
            execute_command("sense -t", temp_result, sizeof(temp_result));
        } else if (strstr(buffer, "GET /pressure") != NULL) {
            execute_command("sense -p", pressure_result, sizeof(pressure_result));
        } else if (strstr(buffer, "GET /humidity") != NULL) {
            execute_command("sense -m", humidity_result, sizeof(humidity_result));
        } else if (strstr(buffer, "GET /gas") != NULL) {
            execute_command("sense -g", gas_result, sizeof(gas_result));
        }

        char response[BUF_SIZE];
        snprintf(response, sizeof(response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html; charset=utf-8\r\n"
                 "\r\n"
                 "<!DOCTYPE HTML>"
                 "<html>"
                 "  <head>"
                 "    <meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
                 "  </head>"
                 "  <body>"
                 "    <h1>ODROID - Web Server</h1>"
                 "    <p>Температура"
                 "      <a href=\"temperature\">"
                 "        <button>Показать</button>"
                 "      </a>"
                 "    </p>"
                 "    <pre>%s</pre>"
                 "    <p>Давление"
                 "      <a href=\"pressure\">"
                 "        <button>Показать</button>"
                 "      </a>"
                 "    </p>"
                 "    <pre>%s</pre>"
                 "    <p>Влажность"
                 "      <a href=\"humidity\">"
                 "        <button>Показать</button>"
                 "      </a>"
                 "    </p>"
                 "    <pre>%s</pre>"
                 "    <p>Газ"
                 "      <a href=\"gas\">"
                 "        <button>Показать</button>"
                 "      </a>"
                 "    </p>"
                 "    <pre>%s</pre>"
                 "  </body>"
                 "</html>", temp_result, pressure_result, humidity_result, gas_result);

        send(client_socket, response, strlen(response), 0);
    }

    close(client_socket);
    free(client);
    pthread_exit(NULL);
}

volatile sig_atomic_t server_running = 1;
int server_socket;

void handle_signal(int sig) {
    if (sig == SIGINT) {
        printf("\nReceived SIGINT. Shutting down server...\n");
        server_running = 0;
        close(server_socket);
    }
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
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding connection");
        exit(EXIT_FAILURE);
    }

    size = sizeof(server_addr);
    printf("=> Looking for clients...\n");

    listen(server_socket, 5);

    signal(SIGINT, handle_signal);

    pthread_t tid;
    while (server_running) {
        client_info *client = malloc(sizeof(client_info));
        client->client_socket = accept(server_socket, (struct sockaddr*)&client->client_addr, &size);

        if (client->client_socket < 0) {
            if (!server_running) {
                // Server is shutting down
                free(client);
                break;
            }
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
    close(server_socket);
    printf("Goodbye...\n");

    return 0;
}
