#ifndef __INC_LINUX_CACHE__
#define __INC_LINUX_CACHE__
#include <sys/queue.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

STAILQ_HEAD(cache_list_head, memory_cache);

struct memory_cache
{
    STAILQ_ENTRY(memory_cache) nodes;
    char* cache_addr;
    int   cache_size;
} ;

typedef struct
{
    struct cache_list_head cache_head;
    int    max_size;
    int    mem_size;
    int    cache_count;
    int    max_count;
} CacheList_t, *CacheList;
typedef struct memory_cache MemoryCache;
CacheList MemoryCacheCreate(int size, int count);
void MemoryCacheReset(CacheList clh);
MemoryCache* MemoryCacheAlloc(CacheList clh);
void MemoryCacheFree(CacheList clh, MemoryCache* mc_free);
MemoryCache* MemoryCacheGet(CacheList clh);
MemoryCache* MemoryCacheNext(MemoryCache* cache);
int MemoryCacheIsEmpty(CacheList clh);
int MemoryCacheIsFull(CacheList clh);
#endif

