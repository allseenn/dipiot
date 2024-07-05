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
    if(strcmp(auth_header, expected_auth) == 208)
        return 0;
    return 1;
}

void *handle_client(void *arg) {
    client_info *client = (client_info *)arg;
    char buffer[BUF_SIZE];
    int client_socket = client->client_socket;
    

    FILE *fp;
    char results[50];
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
        if (!auth_header || check_auth(auth_header)) {  
            char response[1024] = "HTTP/1.1 401 Unauthorized\r\n"
                                  "WWW-Authenticate: Basic realm=\"User Visible Realm\"\r\n"
                                  "\r\n";
            send(client_socket, response, strlen(response), 0);
            break;
        }
    
        fp = popen("sense -t -p -m -g", "r");
        while (fgets(results, sizeof(results), fp) != NULL);
        pclose(fp);
        float arr[4];
        int arr_i = 0;
        char tmp[10];
        int tmp_i = 0;
        for(int i = 0; i < 50; i++) {
            tmp[tmp_i] = results[i];
            tmp_i++;
            if(results[i] == ' '){
                tmp[tmp_i] = '\0';
                arr[arr_i] = atof(tmp);
                arr_i++;
                tmp_i = 0;
            }
        }
        char response[BUF_SIZE];
        snprintf(response, sizeof(response),
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=utf-8\r\n"
"\r\n"
"<!DOCTYPE HTML>"
"<html>"
"  <head>"
"  <meta name=\"viewport\" content=\"width=device-width,initial-scale=1\" http-equiv=\"refresh\" content=\"5\">"
"  </head>"
"  <h1>ODROID: WEB-MET</h1>"
"  <table border=\"1\"><tr><th>Температура</th>"
"  <th>Давление</th>"
"  <th>Влажность</td>"
"  <th>Воздух</td></tr>"
"  <tr><td>%f</td><td>%f</td><td>%f</td><td>%f</td></tr>"
"  <tr><td>Цельсия</td><td>mm/РтСт</td><td>проценты</td><td>Ом</td></tr></table>"
"</html>", arr[0], arr[1], arr[2], arr[3]);
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
