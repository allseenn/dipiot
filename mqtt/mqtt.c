#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "MQTTClient.h"

#define DEFAULT_ADDRESS "192.168.1.8:1883"
#define CLIENTID        "ExampleClientPub"
#define QOS             1
#define TIMEOUT         10000L

void publish_message(MQTTClient client, const char* topic, const char* payload);

int main(int argc, char* argv[])
{
    const char* address = DEFAULT_ADDRESS;
    const char* username = "admin";
    const char* password = "students";
    const char* topics[] = {
        "temperature", "raw_temperature", "humidity", "raw_humidity", 
        "pressure", "gas", "co2_equivalent", "breath_voc_equivalent", 
        "iaq", "static_iaq", "iaq_accuracy", "bsec_status"
    };

    int opt;
    while ((opt = getopt(argc, argv, "i:u:p:")) != -1) {
        switch (opt) {
            case 'i':
                address = optarg;
                break;
            case 'u':
                username = optarg;
                break;
            case 'p':
                password = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-i address] [-u username] [-p password]\n", argv[0]);
                printf("t: temperature\nrt: raw temperature\nhu: humidity\nrhu: raw humidity\np: pressure\n");
                printf("g: gas\nce: co2 equivalent\nbe: breath voc equivalent\n");
                printf("ia: iaq\nis: static iaq\niaq: iaq accuracy\nbs: bsec status\n");
                return -1;
        }
    }

    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    MQTTClient_create(&client, address, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username = username;
    conn_opts.password = password;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove the newline character

        char* token = strtok(buffer, " ");
        int index = 0;

        while (token != NULL && index < sizeof(topics)/sizeof(topics[0])) {
            publish_message(client, topics[index], token);
            token = strtok(NULL, " ");
            index++;
        }
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
