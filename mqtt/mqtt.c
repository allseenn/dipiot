#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"

#define ADDRESS     "192.168.1.8:1883"
#define CLIENTID    "ExampleClientPub"
#define QOS         1
#define TIMEOUT     10000L

void publish_message(MQTTClient client, const char* topic, const char* payload);

int main(int argc, char* argv[])
{
    const char* topics[] = {"temperature", "pressure", "humidity", "gas"};
    char buffer[256];
    char* tokens[4];
    int num_tokens = 0;

    if (argc == 1) {
        // Считываем данные из стандартного ввода
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            char* token = strtok(buffer, " ");
            while (token != NULL && num_tokens < 4) {
                tokens[num_tokens++] = token;
                token = strtok(NULL, " ");
            }
        }
    } else {
        // Считываем данные из аргументов командной строки
        if (argc < 2 || argc > 5) {
            printf("Usage: %s <temperature> [pressure] [humidity] [gas]\n", argv[0]);
            return -1;
        }
        for (int i = 1; i < argc; i++) {
            tokens[num_tokens++] = argv[i];
        }
    }

    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username = "admin";
    conn_opts.password = "students";

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }

    for (int i = 0; i < num_tokens; i++) {
        publish_message(client, topics[i], tokens[i]);
    }

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}

void publish_message(MQTTClient client, const char* topic, const char* payload)
{
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    pubmsg.payload = (void*)payload;
    pubmsg.payloadlen = strlen(payload);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    printf("Waiting for up to %d seconds for publication of %s\n"
           "on topic %s\n",
           (int)(TIMEOUT / 1000), payload, topic);
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("Message with delivery token %d delivered\n", token);
}
