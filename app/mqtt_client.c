#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "thread.h"
#include "mqttClass.h"

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

void main()
{
    MQTT_Session* Session = MQTT_SessionCreate("198.41.30.241", 1883);
    char keep_going = 'c';

    Session->Connect(Session);
    Session->Subscribe(Session, "wuhan/#");

    taskCreate(fetchingWork, Session);

    while (keep_going)
    {
        keep_going = getchar();

        if (keep_going == 'z')
            break;

        if (keep_going == 'a')
            Session->Publish(Session, "wuhan/test", "hello", 5);

    }

    Session->Disconnect(Session);
}

