#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>

#define PORT 8080
#define BUF_SIZE 1024

typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
} client_info;

void *handle_client(void *arg) {
    client_info *client = (client_info *)arg;
    int client_socket = client->client_socket;

    char response[BUF_SIZE];
    snprintf(response, sizeof(response),
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=utf-8\r\n"
"\r\n"
"<!DOCTYPE HTML>"
"<html>"
"  <head>"
"  <meta name=\"viewport\" content=\"width=device-width,initial-scale=1\" http-equiv=\"refresh\" content=\"3\">"
"  </head>"
"  <body>"
"  <h1>ODROID: WEB-MET</h1>"
"  <table border=\"1\" id=\"data-table\">"
"    <tr><th>temp</th>"
"    <th>raw_temp</th>"
"    <th>humidity</th>"
"    <th>raw_hum</th>"
"    <th>press</th>"
"    <th>gas</th>"
"    <th>ceCO2</th>"
"    <th>bVOC</th>"
"    <th>IAQ</th>"
"    <th>SIAQ</th>"
"    <th>IAQ_ACC</th>"
"    <th>status</th>"
"    <th>Dynamic Rad</th>"
"    <th>Static Rad</th></tr>"
"    <tr id=\"data-row\">"
"    <td></td><td></td><td></td><td></td>"
"    <td></td><td></td><td></td><td></td>"
"    <td></td><td></td><td></td><td></td>"
"    <td></td><td></td></tr>"
"  </table>"
"  <script>"
"    function updateData(temp, raw_temp, humidity, raw_hum, press, gas, ceCO2, bVOC, IAQ, SIAQ, IAQ_ACC, status, dynamicRad, staticRad) {"
"        document.querySelector('#data-row td:nth-child(1)').innerText = temp;"
"        document.querySelector('#data-row td:nth-child(2)').innerText = raw_temp;"
"        document.querySelector('#data-row td:nth-child(3)').innerText = humidity;"
"        document.querySelector('#data-row td:nth-child(4)').innerText = raw_hum;"
"        document.querySelector('#data-row td:nth-child(5)').innerText = press;"
"        document.querySelector('#data-row td:nth-child(6)').innerText = gas;"
"        document.querySelector('#data-row td:nth-child(7)').innerText = ceCO2;"
"        document.querySelector('#data-row td:nth-child(8)').innerText = bVOC;"
"        document.querySelector('#data-row td:nth-child(9)').innerText = IAQ;"
"        document.querySelector('#data-row td:nth-child(10)').innerText = SIAQ;"
"        document.querySelector('#data-row td:nth-child(11)').innerText = IAQ_ACC;"
"        document.querySelector('#data-row td:nth-child(12)').innerText = status;"
"        document.querySelector('#data-row td:nth-child(13)').innerText = dynamicRad;"
"        document.querySelector('#data-row td:nth-child(14)').innerText = staticRad;"
"    }"
"  </script>"
"  </body>"
"</html>");

    send(client_socket, response, strlen(response), 0);
    
    while (1) {
        FILE *fp;
        char line[99];

        fp = fopen("/tmp/bsec", "r");
        if (fp == NULL) {
            printf("Error opening file /tmp/bsec\n");
            break;
        }
        
        if (fgets(line, sizeof(line), fp) == NULL) {
            printf("Error reading from file /tmp/bsec\n"); 
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
            break;
        }

        while (fgets(rad_line, sizeof(rad_line), fp_rad) != NULL) {
            sscanf(rad_line, "%d %d", &rad[0], &rad[1]);
        }
        pclose(fp_rad);

        char update[BUF_SIZE];
        snprintf(update, sizeof(update),
"updateData(%.1f, %.1f, %.1f, %.1f, %.0f, %.0f, %.0f, %.2f, %.0f, %.0f, %.0f, %.0f, %d, %d);",
                arr[0], arr[1], arr[2], arr[3],
                arr[4], arr[5], arr[6], arr[7],
                arr[8], arr[9], arr[10], arr[11],
                rad[0], rad[1]);

        // Отправляем только JavaScript код для обновления данных
        send(client_socket, update, strlen(update), 0);
        sleep(3); // Подождем 3 секунд, прежде чем снова отправить обновление
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
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
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
