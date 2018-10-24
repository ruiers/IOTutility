#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "thread.h"
#include "mqttClass.h"
#include "debug.h"
void* fetchingWork(void* arg)
{
    MQTT_Session* Session = (MQTT_Session*) arg;

    MemoryStream topic_and_message =  MemoryStreamCreate();
    MemoryByteArray* topic_or_message = NULL;

    while(1)
    {
        Session->Fetch(Session, topic_and_message);

        if ( topic_and_message->Length > 0 )
        {
            topic_or_message = topic_and_message->GetByteArray(topic_and_message);
            printf("topic   %4d:%s\n", topic_or_message->size, topic_or_message->addr);
            topic_or_message = topic_and_message->NextByteArray(topic_or_message);
            printf("message %4d:%s\n", topic_or_message->size, topic_or_message->addr);
            topic_and_message->EmptyByteArray(topic_and_message);
        }
    }
}

void main(int argc, char** argv)
{
    char hostIPaddr[16] = "198.41.30.241";
    int  hostPortNumber = 1883;
    char *topic_string = "wuhan/test";
    char *message_string = "hello";
    MQTT_Session* Session = NULL;
    char keep_going = 'c';
    int  i = 1;

    for (i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--host"))
        {
            strcpy(hostIPaddr, argv[++i]);
        }

        if(!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port"))
        {
            hostPortNumber = atoi(argv[++i]);
        }

        if(!strcmp(argv[i], "-t") || !strcmp(argv[i], "--topic"))
        {
            topic_string = argv[++i];
        }

        if(!strcmp(argv[i], "-m") || !strcmp(argv[i], "--message"))
        {
            message_string = argv[++i];
        }
    }

    Session = MQTT_SessionCreate(hostIPaddr, hostPortNumber);
    Session->Connect(Session);
    Session->Subscribe(Session, "wuhan/#");

    taskCreate(fetchingWork, Session);

    while (keep_going)
    {
        keep_going = getchar();

        if (keep_going == 'z')
            break;

        if (keep_going == 's')
        {
            Session->Subscribe(Session, "wuhan/#");
        }

        if (keep_going == 'a')
        {
            Session->Publish(Session, topic_string, message_string, strlen(message_string));
        }

    }

    Session->Disconnect(Session);
}

