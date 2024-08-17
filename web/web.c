#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>

#define PORT 8080

// Структура для хранения данных
typedef struct {
    float temp;
    float raw_temp;
    float humidity;
    float raw_hum;
    float press;
    float gas;
    float ceCO2;
    float bVOC;
    float IAQ;
    float SIAQ;
    float IAQ_ACC;
    char status[20];
    float Dynamic_Rad;
    float Static_Rad;
} SensorData;

SensorData sensor_data = {25.0, 24.0, 60.0, 59.0, 1013.0, 0.5, 400.0, 0.1, 50.0, 55.0, 45.0, "ОК", 1.0, 1.0};

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);

    char buffer[1024];
    read(client_socket, buffer, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0'; // Убедимся, что строка заканчивается нулевым символом

    if (strstr(buffer, "GET /data") != NULL) {
        // Обработка запроса на данные
        char json_response[256];
        snprintf(json_response, sizeof(json_response),
                 "{\"temp\":%.1f,\"raw_temp\":%.1f,\"humidity\":%.1f,\"raw_hum\":%.1f,\"press\":%.1f,\"gas\":%.1f,\"ceCO2\":%.1f,\"bVOC\":%.1f,\"IAQ\":%.1f,\"SIAQ\":%.1f,\"IAQ_ACC\":%.1f,\"status\":\"%s\",\"Dynamic_Rad\":%.1f,\"Static_Rad\":%.1f}",
                 sensor_data.temp, sensor_data.raw_temp, sensor_data.humidity, sensor_data.raw_hum,
                 sensor_data.press, sensor_data.gas, sensor_data.ceCO2, sensor_data.bVOC,
                 sensor_data.IAQ, sensor_data.SIAQ, sensor_data.IAQ_ACC, sensor_data.status,
                 sensor_data.Dynamic_Rad, sensor_data.Static_Rad);

        char response[512];
        snprintf(response, sizeof(response),
                 "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n%s", json_response);
        send(client_socket, response, strlen(response), 0);
    } else {
        // Отправка основного HTML-ответа
        char *response = "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/html; charset=utf-8\r\n"
                         "\r\n"
                         "<!DOCTYPE HTML>"
                         "<html>"
                         "<head>"
                         "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
                         "<script src=\"https://code.jquery.com/jquery-3.6.0.min.js\"></script>"
                         "<script>"
                         "function fetchData() {"
                         "    $.get('/data', function(data) {"
                         "        var json = JSON.parse(data);"
                         "        $('#temp').text(json.temp);"
                         "        $('#raw_temp').text(json.raw_temp);"
                         "        $('#humidity').text(json.humidity);"
                         "        $('#raw_hum').text(json.raw_hum);"
                         "        $('#press').text(json.press);"
                         "        $('#gas').text(json.gas);"
                         "        $('#ceCO2').text(json.ceCO2);"
                         "        $('#bVOC').text(json.bVOC);"
                         "        $('#IAQ').text(json.IAQ);"
                         "        $('#SIAQ').text(json.SIAQ);"
                         "        $('#IAQ_ACC').text(json.IAQ_ACC);"
                         "        $('#status').text(json.status);"
                         "        $('#Dynamic_Rad').text(json.Dynamic_Rad);"
  
                         "        $('#Static_Rad').text(json.Static_Rad);"
                         "    });"
                         "}"
                         "setInterval(fetchData, 3000);"
                         "</script>"
                         "</head>"
                         "<body>"
                         "<h1>Данные датчиков</h1>"
                         "<table border='1' id='data-table'>"
                         "    <tr>"
                         "        <th>Температура</th>"
                         "        <th>Температура (сырая)</th>"
                         "        <th>Влажность</th>"
                         "        <th>Влажность (сырая)</th>"
                         "        <th>Давление</th>"
                         "        <th>Газ</th>"
                         "        <th>СО2 эквивалент</th>"
                         "        <th>Летучие органические вещества</th>"
                         "        <th>IAQ</th>"
                         "        <th>SIAQ</th>"
                         "        <th>IAQ (скорректированный)</th>"
                         "        <th>Статус</th>"
                         "        <th>Динамическое излучение</th>"
                         "        <th>Статистическое излучение</th>"
                         "    </tr>"
                         "    <tr>"
                         "        <td id='temp'></td>"
                         "        <td id='raw_temp'></td>"
                         "        <td id='humidity'></td>"
                         "        <td id='raw_hum'></td>"
                         "        <td id='press'></td>"
                         "        <td id='gas'></td>"
                         "        <td id='ceCO2'></td>"
                         "        <td id='bVOC'></td>"
                         "        <td id='IAQ'></td>"
                         "        <td id='SIAQ'></td>"
                         "        <td id='IAQ_ACC'></td>"
                         "        <td id='status'></td>"
                         "        <td id='Dynamic_Rad'></td>"
                         "        <td id='Static_Rad'></td>"
                         "    </tr>"
                         "</table>"
                         "</body>"
                         "</html>";

        // Отправка основного HTML-ответа клиенту 
        send(client_socket, response, strlen(response), 0);
    }

    close(client_socket);
    return NULL;
}

int main() {
    int server_socket, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Ошибка при создании сокета");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Ошибка при привязке сокета");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Ошибка при прослушивании");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Сервер запущен на порту %d\n", PORT);

    while (1) {
        int *new_sock = malloc(sizeof(int));
        if (new_sock == NULL) {
            perror("Ошибка при выделении памяти");
            continue;
        }

        *new_sock = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (*new_sock < 0) {
            perror("Ошибка при принятии подключения");
            free(new_sock);
            continue;
        }

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, (void *)new_sock) != 0) {
            perror("Ошибка при создании потока");
            close(*new_sock);
            free(new_sock);
            continue;
        }

        pthread_detach(client_thread); // Отсоединяем поток для автоматического освобождения ресурсов
    }

    close(server_socket);
    return 0;
}
