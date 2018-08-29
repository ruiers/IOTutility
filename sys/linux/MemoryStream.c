#include "linux/MemoryStream.h"

void* MemoryStreamWrite(MemoryStream this, char* data_addr, int data_len)
{
    if ((data_addr == NULL) || (data_len > (this->Capacity - this->Position)))
        return NULL;

    if (data_len < 8)
        for (int i = 0; i < data_len; i++)
            *(this->Memory + this->Position + i) = *(data_addr + i);
    else
        memcpy(this->Memory + this->Position, data_addr, data_len);

    this->Length += data_len;
    this->Position += data_len;

    return 0;
}

void* MemoryStreamRead(MemoryStream this, char* data_addr, int data_len)
{
    if ((data_addr == NULL) || (data_len > (this->Length)))
        return NULL;

    memcpy(data_addr, this->Memory + this->Position - data_len, data_len);

    this->Length -= data_len;
    this->Position -= data_len;

    return 0;
}

MemoryStream MemoryStreamAlloc(int size)
{
    MemoryStream ms = calloc(1, sizeof(MemoryStream_T));

    ms->Memory = malloc(size);
    memset(ms->Memory, 0x0, size);

    ms->this = ms;
    ms->Capacity = size;
    ms->Length   = 0;
    ms->Position = 0;

    ms->Write = MemoryStreamWrite;
    ms->Read = MemoryStreamRead;

    return ms;
}

MemoryByteArray* MemoryStreamAddByteArray(MemoryStream ms, int data_len)
{
    MemoryByteArray* insert = malloc(sizeof(MemoryByteArray));
    insert->addr = calloc(1, data_len);
    insert->size = data_len;

    STAILQ_INSERT_TAIL(&ms->head, insert, nodes);
    ms->Length += data_len;
    ms->Position += data_len;

    return insert;
}

void MemoryStreamDelByteArray(MemoryStream ms, MemoryByteArray* array)
{
    if (array == NULL)
        return;

    if (ms->Length == 0)
        return;

    ms->Length -= array->size;
    ms->Position -= array->size;

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
    ms->Position = 0;
}

MemoryStream MemoryStreamCreate(void)
{
    MemoryStream ms = calloc(1, sizeof(MemoryStream_T));
    STAILQ_INIT(&ms->head);

    ms->this     = ms;
    ms->Capacity = -1;
    ms->Length   = 0;
    ms->Position = 0;

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