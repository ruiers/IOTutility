#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "MemoryStream.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

int MemoryStreamWrite(MemoryStream this, char* data_addr, int data_len)
{
    int i = 0;
    int Capacity = MIN(data_len, this->Length - this->InPos + this->OutPos);
    int Left     = MIN(Capacity, this->Length - (this->InPos & (this->Length -1)));

    memcpy(this->Memory + (this->InPos & (this->Length -1)), data_addr, Left);
    memcpy(this->Memory, data_addr + Left, Capacity - Left);

    this->InPos += Capacity;

    return Capacity;
}

int MemoryStreamRead(MemoryStream this, char* data_addr, int data_len)
{
    int Capacity = MIN(data_len, this->InPos - this->OutPos);
    int Left     = MIN(Capacity, this->Length - (this->OutPos & (this->Length - 1)));

    memcpy(data_addr, this->Memory + (this->OutPos & (this->Length - 1)), Left);
    memcpy(data_addr + Left, this->Memory, Capacity - Left);

    return Capacity;
}

MemoryStream MemoryStreamAlloc(int size)
{
    MemoryStream ms = calloc(1, sizeof(MemoryStream_T));

    ms->Memory = malloc(size);
    memset(ms->Memory, 0x0, size);

    ms->this   = ms;
    ms->Length = size;
    ms->InPos  = 0;
    ms->OutPos = 0;

    ms->Write  = MemoryStreamWrite;
    ms->Read   = MemoryStreamRead;

    return ms;
}

MemoryByteArray* MemoryStreamAddByteArray(MemoryStream ms, char* data_addr, int data_len)
{
    MemoryByteArray* insert = malloc(sizeof(MemoryByteArray));
    insert->addr = calloc(1, data_len);
    insert->size = data_len;

    memcpy(insert->addr, data_addr, data_len);

    STAILQ_INSERT_TAIL(&ms->head, insert, nodes);
    ms->Length += data_len;
    ms->Count  += 1;

    return insert;
}

void MemoryStreamDelByteArray(MemoryStream ms, MemoryByteArray* array)
{
    if (array == NULL)
        return;

    if (ms->Length == 0)
        return;

    ms->Length -= array->size;
    ms->Count  -= 1;

    STAILQ_REMOVE(&ms->head, array, MemoryBytes, nodes);

    free(array->addr);
    free(array);
}

MemoryByteArray* MemoryStreamGetByteArray(MemoryStream ms)
{
    return STAILQ_FIRST(&ms->head);
}

MemoryByteArray* MemoryStreamNextByteArray(MemoryByteArray *next)
{
    return STAILQ_NEXT(next, nodes);
}

void MemoryStreamEmptyByteArray(MemoryStream ms)
{
    while (!STAILQ_EMPTY(&ms->head))
    {
        MemoryByteArray* remove = STAILQ_FIRST(&ms->head);
        STAILQ_REMOVE_HEAD(&ms->head, nodes);
        free(remove->addr);
        free(remove);
    }

    ms->Length = 0;
    ms->Count  = 0;
}

MemoryStream MemoryStreamCreate(void)
{
    MemoryStream ms = calloc(1, sizeof(MemoryStream_T));
    STAILQ_INIT(&ms->head);

    ms->this     = ms;
    ms->Count    = 0;
    ms->Length   = 0;

    ms->AddByteArray    = MemoryStreamAddByteArray;
    ms->DeleteByteArray = MemoryStreamDelByteArray;
    ms->GetByteArray    = MemoryStreamGetByteArray;
    ms->NextByteArray   = MemoryStreamNextByteArray;
    ms->EmptyByteArray  = MemoryStreamEmptyByteArray;

    return ms;
}

void MemoryStreamDelete(MemoryStream self)
{
    if (self)
    {
        if (self->Memory)
            free(self->Memory);
        free(self);
    }

}