#include <unistd.h>
#include <stdlib.h>
#include "IEC61850_9_2.h"

IEC61850_9_2* IEC61850_9_2_Dump(char* org_data, int org_len)
{
    IEC61850_9_2* this = (IEC61850_9_2*) calloc(1, sizeof(IEC61850_9_2));

    this->data = MemoryStreamAlloc(org_len);
    this->info = (IEC61850_9_2_HEAD*) this->data->Memory;

    this->data->Write(this->data, org_data, org_len);

    return this;
}

IEC61850_9_2* IEC61850_9_2_Create(int Length)
{
    IEC61850_9_2* this = (IEC61850_9_2*) calloc(1, sizeof(IEC61850_9_2));
    IEC61850_9_2_HEAD* Head = NULL;

    this->data = MemoryStreamCreate();
    Head = (IEC61850_9_2_HEAD *) malloc(sizeof(IEC61850_9_2_HEAD));
    this->data->AddByteArray(this->data, (char*) Head, sizeof(IEC61850_9_2_HEAD));
    this->info = (IEC61850_9_2_HEAD *) this->data->GetByteArray(this->data);

    return this;
}
