#ifndef __INC_LINUX_MEMORYSTREAM__
#define __INC_LINUX_MEMORYSTREAM__
#include <sys/queue.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define is_power_of_2(x)	((x) != 0 && (((x) & ((x) - 1)) == 0))

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
    int    Count;
    int    Length;
    int    InPos;
    int    OutPos;

    int (*Write) (struct MemoryStream_Head* this, char* data_addr, int data_len);
    int (*Read)  (struct MemoryStream_Head* this, char* data_addr, int data_len);

    MemoryByteArray* (*AddByteArray)  (struct MemoryStream_Head* this, char* data_addr, int data_len);
    void (*DeleteByteArray) (struct MemoryStream_Head* this, MemoryByteArray* array);
    MemoryByteArray* (*GetByteArray)  (struct MemoryStream_Head* this);
    MemoryByteArray* (*NextByteArray) (MemoryByteArray *next);
    void(*EmptyByteArray) (struct MemoryStream_Head* this);

} MemoryStream_T, *MemoryStream;

MemoryStream MemoryStreamAlloc(int size);
MemoryStream MemoryStreamCreate(void);
void MemoryStreamDelete(MemoryStream self);
#endif
