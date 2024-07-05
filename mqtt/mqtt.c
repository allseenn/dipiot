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
    if (argc != 5) {
        printf("Usage: %s <temperature> <pressure> <humidity> <gas>\n", argv[0]);
        return -1;
    }

    const char* temperature = argv[1];
    const char* pressure = argv[2];
    const char* humidity = argv[3];
    const char* gas = argv[4];

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

    publish_message(client, "temperature", temperature);
    publish_message(client, "pressure", pressure);
    publish_message(client, "humidity", humidity);
    publish_message(client, "gas", gas);

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
