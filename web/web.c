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

void send_html_response(int client_socket) {
    char response[BUF_SIZE];
    snprintf(response, sizeof(response),
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=utf-8\r\n"
"\r\n"
"<!DOCTYPE HTML>"
"<html>"
"  <head>"
"  <meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
"  <script>"
"  function updateData() {"
"    fetch('/data').then(response => response.json()).then(data => {"
"      document.getElementById('temp').textContent = data.temp.toFixed(1);"
"      document.getElementById('raw_temp').textContent = data.raw_temp.toFixed(1);"
"      document.getElementById('humidity').textContent = data.humidity.toFixed(1);"
"      document.getElementById('raw_hum').textContent = data.raw_hum.toFixed(1);"
"      document.getElementById('press').textContent = data.press.toFixed(0);"
"      document.getElementById('gas').textContent = data.gas.toFixed(0);"
"      document.getElementById('ceCO2').textContent = data.ceCO2.toFixed(0);"
"      document.getElementById('bVOC').textContent = data.bVOC.toFixed(2);"
"      document.getElementById('IAQ').textContent = data.IAQ.toFixed(0);"
"      document.getElementById('SIAQ').textContent = data.SIAQ.toFixed(0);"
"      document.getElementById('IAQ_ACC').textContent = data.IAQ_ACC.toFixed(0);"
"      document.getElementById('status').textContent = data.status.toFixed(0);"
"      document.getElementById('dyn_rad').textContent = data.dyn_rad.toFixed(0);"
"      document.getElementById('stat_rad').textContent = data.stat_rad.toFixed(0);"
"    });"
"  }"
"  setInterval(updateData, 3000);"
"  </script>"
"  </head>"
"  <body onload=\"updateData()\">"
"  <h1>ODROID: WEB-MET</h1>"
"  <table border=\"1\">"
"  <tr><th>temp</th><th>raw_temp</th><th>humidity</th><th>raw_hum</th>"
"      <th>press</th><th>gas</th><th>ceCO2</th><th>bVOC</th>"
"      <th>IAQ</th><th>SIAQ</th><th>IAQ_ACC</th><th>status</th>"
"      <th>Dynamic Rad</th><th>Static Rad</th></tr>"
"  <tr>"
"    <td id=\"temp\"></td>"
"    <td id=\"raw_temp\"></td>"
"    <td id=\"humidity\"></td>"
"    <td id=\"raw_hum\"></td>"
"    <td id=\"press\"></td>"
"    <td id=\"gas\"></td>"
"    <td id=\"ceCO2\"></td>"
"    <td id=\"bVOC\"></td>"
"    <td id=\"IAQ\"></td>"
"    <td id=\"SIAQ\"></td>"
"    <td id=\"IAQ_ACC\"></td>"
"    <td id=\"status\"></td>"
"    <td id=\"dyn_rad\"></td>"
"    <td id=\"stat_rad\"></td>"
"  </tr>"
"  <tr><td>C&deg;</td><td>C&deg;</td><td>&percnt;</td><td>&percnt;</td>"
"      <td>mmHg</td><td>KOM</td><td>ppm</td><td>ppm</td>"
"      <td>index</td><td>index</td><td>num</td><td>num</td>"
"      <td>&mu;R/h</td><td>&mu;R/h</td></tr>"
"  </table>"
"  </body>"
"</html>");
    send(client_socket, response, strlen(response), 0);
}

void send_json_data(int client_socket) {
    FILE *fp;
    char line[99];

    fp = fopen("/tmp/bsec", "r");
    if (fp == NULL) {
        printf("Error opening file /tmp/bsec\n");
        return;
    }
    
    if (fgets(line, sizeof(line), fp) == NULL) {
        printf("Error reading from file /tmp/bsec\n");
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

    sprintf(cmd, "./rad.sh");
    fp_rad = popen(cmd, "r");
    if (fp_rad == NULL) {
        printf("Failed to run command\n");
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
        "\"press\": %.0f, \"gas\": %.0f, \"ceCO2\": %.0f, \"bVOC\": %.2f,"
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
        perror("Error receiving request");
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

