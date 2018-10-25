#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "mqttClass.h"
#include "TcpClass.h"

int client_id_generate(unsigned char* client_id, const char *id_base)
{
    int len;
    char hostname[256];

    if (client_id == NULL)
    {
        client_id = malloc(len);

        if(!client_id)
        {
            return -1;
        }
    }
    else
    {
        hostname[0] = '\0';
        gethostname(hostname, 256);
        hostname[255] = '\0';
        len = strlen(id_base) + strlen("|-") + 6 + strlen(hostname);
        snprintf(client_id, len, "%s|%d-%s", id_base, getpid(), hostname);

        if(strlen(client_id) > MOSQ_MQTT_ID_MAX_LENGTH)
        {
            /* Enforce maximum client id length of 23 characters */
            client_id[MOSQ_MQTT_ID_MAX_LENGTH] = '\0';
        }
    }

    return strlen(client_id);
}

int encode_integer_to_length(unsigned char* remaining_length, int value)
{
    unsigned char std_bytes[4];
    int  bytes_total = 0, X = value, encodedByte = 0;

    memset(std_bytes, 0, sizeof(std_bytes));

    do
    {
        encodedByte = X % 128;
        X = X / 128;

        if ( X > 0)
            encodedByte = encodedByte | 0x80;

        remaining_length[bytes_total++] = encodedByte;
    }
    while (X > 0);

    return (bytes_total);
}

int decode_length_to_interger(unsigned char* remaining_length, int *value)
{
    unsigned char encodedByte;
    int  bytes_total = 0, multiplier = 1;

    *value = 0;

    do
    {
        encodedByte = remaining_length[bytes_total++];
        *value += (encodedByte & 127) * multiplier;
        multiplier *= 128;
        if ( multiplier > 128*128*128 )
            return -1;
    }
    while ( (encodedByte & 128) != 0 );

    return bytes_total;
}

int setTcpClientTimeout(int sock_fd, int ns_timeout)
{
    struct timeval timeout;
    int status = 0;
    timeout.tv_sec  = ns_timeout;
    timeout.tv_usec = 0;

    status = setsockopt(sock_fd,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));

    if (status != 0)
        return status;

    status = setsockopt(sock_fd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));

    return status;
}

MQTT_ControlPacket* MQTT_ControlPacketCreate(int PacketType)
{
    MQTT_ControlPacket* packet = calloc(1, sizeof(MQTT_ControlPacket));
    int   length_of_payload = 0;
    int   remaining_length_bytes = 0;
    void* fixedHeader = NULL;
    void* variableHeader = NULL;
    void* PayloadStart = NULL;

    packet->ControlPacket = MemoryStreamCreate();
    packet->PacketType = PacketType;

    switch (packet->PacketType)
    {
    case CONNECT:
        variableHeader = (VariableHeader_Connect*) calloc(1, sizeof(VariableHeader_Connect));
        ((VariableHeader_Connect*) variableHeader)->protocol_length = htons(sizeof(PROTOCOL_NAME_v311) - 1);
        memcpy(((VariableHeader_Connect*) variableHeader)->protocol_name, PROTOCOL_NAME_v311, sizeof(PROTOCOL_NAME_v311) - 1);
        ((VariableHeader_Connect*) variableHeader)->protocol_level = PROTOCOL_VERSION_v311;
        ((VariableHeader_Connect*) variableHeader)->connect_flags = 2;
        ((VariableHeader_Connect*) variableHeader)->keep_alive = htons(60);

        PayloadStart = (Payload*) calloc(1, 23 + sizeof(short));
        length_of_payload = client_id_generate(((Payload*) PayloadStart)->message, "mosqpub");
        ((Payload*) PayloadStart)->length = htons(length_of_payload);

        fixedHeader = (FixedHeader*) calloc(1, 5);
        ((FixedHeader*) fixedHeader)->type_and_flag = packet->PacketType;
        packet->PayloadLength = sizeof(VariableHeader_Connect) + length_of_payload + sizeof(short);
        packet->FixedHeader = packet->ControlPacket->AddByteArray(packet->ControlPacket, (char *) fixedHeader, sizeof(FixedHeader));
        packet->VariableHeader = packet->ControlPacket->AddByteArray(packet->ControlPacket, (char *) variableHeader, sizeof(VariableHeader_Connect));
        packet->PayloadStart = packet->ControlPacket->AddByteArray(packet->ControlPacket, (char *) PayloadStart, length_of_payload + sizeof(short));
        break;

    case PUBLISH:
    case SUBSCRIBE:
    case DISCONNECT:
    case PINGREQ:
        fixedHeader = (FixedHeader*) calloc(1, 5);
        ((FixedHeader*) fixedHeader)->type_and_flag = packet->PacketType;
        ((FixedHeader*) fixedHeader)->remaining_length[0] = 0;

        packet->FixedHeader = packet->ControlPacket->AddByteArray(packet->ControlPacket, (char *) fixedHeader, sizeof(FixedHeader));
        break;

    default:
        break;
    }

    return packet;
}

char* MQTT_ControlPacketGetPacketData(MQTT_ControlPacket* this)
{
    MemoryByteArray* array = this->ControlPacket->GetByteArray(this->ControlPacket);
    int pos = 0, remaining_length_bytes = 0;

    remaining_length_bytes = encode_integer_to_length( ((char *) this->FixedHeader->addr) + 1, this->PayloadLength);
    this->PacketLength = this->PayloadLength + remaining_length_bytes + 1;
    this->FixedHeader->size = remaining_length_bytes + 1;
    this->PacketData = calloc(1, this->PacketLength);

    while(array != NULL)
    {
        memcpy(this->PacketData + pos, array->addr, array->size);
        pos += array->size;
        array = this->ControlPacket->NextByteArray(array);
    }

    return this->PacketData;
}

int MQTT_ControlPacketSetTopic(MQTT_ControlPacket* this, char* topic_string, int topic_length)
{
    Payload* topic_payload = calloc(1, sizeof(short) + topic_length);

    topic_payload->length = htons(topic_length);
    memcpy(topic_payload->message, topic_string, topic_length);
    this->VariableHeader = this->ControlPacket->AddByteArray(this->ControlPacket, (char *) topic_payload, sizeof(short) + topic_length);
    this->PayloadLength  += this->VariableHeader->size;
}

int MQTT_ControlPacketSetMessage(MQTT_ControlPacket* this, char* msg_string, int msg_length)
{
    this->PayloadStart  = this->ControlPacket->AddByteArray(this->ControlPacket, (char *) msg_string, msg_length);
    this->PayloadLength += this->PayloadStart->size;
}

int MQTT_SessionConnect(MQTT_Session* this)
{
    MQTT_ControlPacket*  mqttConnect = MQTT_ControlPacketCreate(CONNECT);
    MQTT_ACKPacket       mqttACK;

    this->Status = STA_CONNECTING;
    MQTT_ControlPacketGetPacketData(mqttConnect);
    setTcpClientTimeout(this->Session->Client, 60);
    this->Session->Send(this->Session, mqttConnect->PacketData, mqttConnect->PacketLength);
    /*
    this->Session->Receive(this->Session, (char *) &mqttACK, sizeof(MQTT_ACKPacket));

    if ((mqttACK.type_and_flag == CONNACK) && (mqttACK.ack_code[1] == CONNACK_ACCEPTED))
        this->Status = STA_CONNECTED;
    else
        this->Status = STA_CONNECT_NO_ACK;
    */
}

int MQTT_SessionDisonnect(MQTT_Session* this)
{
    MQTT_ControlPacket*  mqttDisconnect = MQTT_ControlPacketCreate(DISCONNECT);

    MQTT_ControlPacketGetPacketData(mqttDisconnect);
    this->Session->Send(this->Session, mqttDisconnect->PacketData, mqttDisconnect->PacketLength);
    this->Session->Disconnect(this->Session);
}

int MQTT_SessionPublish(MQTT_Session* this, char* topic, char* message, int length)
{
    MQTT_ControlPacket*  mqttPublish = MQTT_ControlPacketCreate(PUBLISH);
    MQTT_ACKPacket       mqttACK;
    int                  mqttStatus;

    if (this->Status != STA_CONNECTED)
        return -1;

    this->Status = STA_PUBLISHING;
    MQTT_ControlPacketSetTopic(mqttPublish, topic, strlen(topic));
    MQTT_ControlPacketSetMessage(mqttPublish, message, length);
    MQTT_ControlPacketGetPacketData(mqttPublish);
    mqttStatus = this->Session->Send(this->Session, mqttPublish->PacketData, mqttPublish->PacketLength);

    if (mqttStatus == mqttPublish->PacketLength)
    {
        this->Status = STA_CONNECTED;
    }
}

int MQTT_SessionSubscribe(MQTT_Session* this, char* topic)
{
    MQTT_ControlPacket*  mqttSubscribe = MQTT_ControlPacketCreate(SUBSCRIBE);
    MQTT_ACKPacket       mqttACK;
    char                 Requested_QoS = (0);
    short                message_identifier = htons(1);
    short                topic_len = htons(strlen(topic));

    if (this->Status != STA_CONNECTED)
        return -1;

    this->Status = STA_SUBSCRIBING;
    *(mqttSubscribe->FixedHeader->addr) |= 0x02;
    MQTT_ControlPacketSetMessage(mqttSubscribe, (char *) &message_identifier, sizeof(short));
    MQTT_ControlPacketSetMessage(mqttSubscribe, (char *) &topic_len, sizeof(short));
    MQTT_ControlPacketSetMessage(mqttSubscribe, topic, strlen(topic));
    MQTT_ControlPacketSetMessage(mqttSubscribe, &Requested_QoS, 1);
    MQTT_ControlPacketGetPacketData(mqttSubscribe);
    this->Session->Send(this->Session, mqttSubscribe->PacketData, mqttSubscribe->PacketLength);
    // this->Session->Receive(this->Session, (char *) &mqttACK, sizeof(MQTT_ACKPacket));
    // FixMe I do not know why it is blocked here.
    this->Status = STA_CONNECTED;
}

int MQTT_SessionPingReq(MQTT_Session* this)
{
    MQTT_ControlPacket*  mqttPingReq = MQTT_ControlPacketCreate(PINGREQ);
    MQTT_ACKPacket       mqttACK;

    if (this->Status != STA_CONNECTED)
        return -1;

    this->Status = STA_PINGREQING;

    MQTT_ControlPacketGetPacketData(mqttPingReq);
    this->Session->Send(this->Session, mqttPingReq->PacketData, mqttPingReq->PacketLength);
    /*
    this->Session->Receive(this->Session, (char *) &mqttACK, sizeof(MQTT_ACKPacket));

    if (mqttACK.type_and_flag == PINGRESP)
        this->Status = STA_CONNECTED;
    else
        this->Status = STA_WAITING_ACK;
    */
}

int MQTT_SessionHandleCommand(unsigned char *cmd_data, int cmd_data_len, MemoryStream topMsg)
{
    int total_len = cmd_data_len, remain_len = 0, remain_len_bytes = 0;
    int topic_len = 0, message_len = 0;
    MQTT_ACKPacket *ackPacket = (MQTT_ACKPacket *) cmd_data;
    MemoryByteArray* topic = NULL, *message = NULL;

    switch ( ackPacket->type_and_flag & 0xF0)
    {
    case PUBACK:
        break;
    case CONNACK:
        if (ackPacket->ack_code[1] == CONNACK_ACCEPTED)
            return STA_CONNECTED;
        break;
    case PUBLISH:
        remain_len_bytes = decode_length_to_interger(cmd_data + 1, &remain_len);
        topic_len = htons(*((unsigned short *) (cmd_data + 1 + remain_len_bytes)));
        message_len = total_len - (1 + remain_len_bytes + 2 + topic_len);

        topic = topMsg->AddByteArray(topMsg, cmd_data + 1 + remain_len_bytes + 2, topic_len);
        message = topMsg->AddByteArray(topMsg, cmd_data + total_len - message_len, message_len);
        break;
    default:
        return STA_CREATED;
    }

    return STA_CONNECTED;
}

int MQTT_SessionFetch(MQTT_Session* this, MemoryStream topMsg)
{
    unsigned char tcp_data[1024];
    int           tcp_data_len = 0;
    int           cmd_data_len = 0;
    int           cmd_data_offset = 0;

    if (this->Status == -1)
        return -1;

    tcp_data_len = this->Session->Receive(this->Session, tcp_data, sizeof(tcp_data));

    while(tcp_data_len > 0)
    {
        switch(tcp_data[cmd_data_offset] & 0xF0)
        {
        case CONNACK:
        case PUBACK:
            cmd_data_len = 4;
            break;
        case PUBLISH:
            cmd_data_len += decode_length_to_interger(tcp_data + cmd_data_offset + 1, &cmd_data_len) + 1;
            break;
        default:
            break;
        }

        if (cmd_data_len < 4)
        {
            break;
        }

        this->Status = MQTT_SessionHandleCommand(tcp_data + cmd_data_offset, cmd_data_len, topMsg);
        tcp_data_len -= cmd_data_len;
        cmd_data_offset += cmd_data_len;
        cmd_data_len = 0;
    }
}

MQTT_Session* MQTT_SessionCreate(char* ipStr, int portNum)
{
    MQTT_Session* session = calloc(1, sizeof(MQTT_Session));

    session->ServerPortNumber = portNum;
    strncpy(session->ServerIPString, ipStr, strlen(ipStr));

    session->Session = tcpClientCreate(session->ServerIPString, session->ServerPortNumber);

    session->Connect    = MQTT_SessionConnect;
    session->Disconnect = MQTT_SessionDisonnect;
    session->Publish    = MQTT_SessionPublish;
    session->Subscribe  = MQTT_SessionSubscribe;
    session->Fetch      = MQTT_SessionFetch;
    session->Status     = STA_CREATED;

    return session;
}

MQTT_Session* MQTT_ServerWaitForSession(MQTT_Server* this)
{
    MQTT_Session* session = calloc(1, sizeof(MQTT_Session));

    session->Session = this->listener->AcceptTcpClient(this->listener);

    return session;
}

MQTT_ControlPacket* MQTT_ServerACKForSession(MQTT_Server* this, MQTT_Session* session)
{
    char data[1500];
    MQTT_ControlPacket* Packet = calloc(1, sizeof(MQTT_ControlPacket));
    MQTT_ACKPacket ack;

    Packet->PacketLength = session->Session->Receive(session->Session, data, sizeof(data));
    Packet->ControlPacket = MemoryStreamCreate();
    Packet->PacketType = *(data);
    Packet->FixedHeader = Packet->ControlPacket->AddByteArray(Packet->ControlPacket, data, 1 + decode_length_to_interger(data + 1, &Packet->RemainLength));
    Packet->ControlPacket->Memory = malloc(Packet->PacketLength);
    memcpy(Packet->ControlPacket->Memory, data, Packet->PacketLength);

    switch (Packet->PacketType)
    {
    case PINGREQ:
        ack.type_and_flag = PINGRESP;
        break;
    case CONNECT:
        ack.type_and_flag = CONNACK;
        ack.remaining_length = 2;
        ack.ack_code[1] = CONNACK_ACCEPTED;
        this->numSession++;
        break;
    case DISCONNECT:
        ack.type_and_flag = DISCONNECT;
        ack.remaining_length = 2;
        this->numSession--;
        break;
    case PUBLISH:
        Packet->VariableHeader = Packet->ControlPacket->AddByteArray(Packet->ControlPacket, data + Packet->PacketLength - Packet->RemainLength,
                                 data[Packet->PacketLength - Packet->RemainLength + 1] + sizeof(short));
        Packet->PayloadStart = Packet->ControlPacket->AddByteArray(Packet->ControlPacket, data + Packet->FixedHeader->size + Packet->VariableHeader->size, Packet->RemainLength - Packet->VariableHeader->size);
        ack.type_and_flag = PUBACK;
        break;
    }

    if( sizeof(ack) == session->Session->Send(session->Session, (char *) &ack, sizeof(ack)))
        return Packet;
    else
        return NULL;
}

MQTT_Server* MQTT_ServerCreate(char* ipStr, int portNum)
{
    MQTT_Server* server = calloc(1, sizeof(MQTT_Server));

    server->WaitForSession = MQTT_ServerWaitForSession;
    server->ACKForSession  = MQTT_ServerACKForSession;
    server->listener   = tcpListenerCreate(ipStr, portNum);
    server->numSession = 0;

    server->listener->Start(server->listener);

    return server;
}
