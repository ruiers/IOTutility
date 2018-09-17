#include "MemoryStream.h"

typedef struct iec61850_9_2_t
{
    char  dstMacAddr[6];
    char  srcMacAddr[6];
    short PriorityTag;
    short vlanID;
    short EtherType;
    short appID;
    short PDULength;
    short Reserved1;
    short Reserved2;
    char  APDU[0];
} IEC61850_9_2_HEAD, *IEC61850_9_2_H;

typedef struct iec61850_9_2
{
    IEC61850_9_2_HEAD *info;
    MemoryStream       data;
} IEC61850_9_2;