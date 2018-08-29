#ifndef __INC_LINUX_MEMORYSTREAM__
#define __INC_LINUX_MEMORYSTREAM__
#include <sys/queue.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

STAILQ_HEAD(memory_list_head, MemoryBytes);

struct MemoryBytes
{
    STAILQ_ENTRY(MemoryBytes) nodes;
    char* addr;
    int   size;
} ;

typedef struct MemoryBytes MemoryByteArray;

typedef struct MemoryStream_Head
{
    struct MemoryStream_Head* this;
    struct memory_list_head head;

    char*  Memory;
    int    Capacity;
    int    Length;
    int    Position;

    void* (*Write) (struct MemoryStream_Head* this, char* data_addr, int data_len);
    void* (*Read)  (struct MemoryStream_Head* this, char* data_addr, int data_len);

    MemoryByteArray* (*AddByteArray)  (struct MemoryStream_Head* this, int data_len);
    MemoryByteArray* (*GetByteArray)  (struct MemoryStream_Head* this);
    MemoryByteArray* (*NextByteArray) (MemoryByteArray *next);
    void(*EmptyByteArray) (struct MemoryStream_Head* this);

} MemoryStream_T, *MemoryStream;

MemoryStream MemoryStreamAlloc(int size);
MemoryStream MemoryStreamCreate(void);
void MemoryStreamDelete(MemoryStream self);
#endif
