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
    const char* topics[] = {"temperature", "pressure", "humidity", "gas"};
    char buffer[256];
    char* tokens[4];
    int num_tokens = 0;

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
                fprintf(stderr, "Usage: %s [-i address] [-u username] [-p password] <temperature> [pressure] [humidity] [gas]\n", argv[0]);
                return -1;
        }
    }

    if (optind < argc) {
        for (int i = optind; i < argc && num_tokens < 4; i++) {
            tokens[num_tokens++] = argv[i];
        }
    } else {
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            char* token = strtok(buffer, " ");
            while (token != NULL && num_tokens < 4) {
                tokens[num_tokens++] = token;
                token = strtok(NULL, " ");
            }
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
