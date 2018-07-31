#include <sys/queue.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

struct memory_cache
{
    STAILQ_ENTRY(memory_cache) nodes;
    char* cache_addr;
    int   cache_size;
} ;

typedef struct cache_list_head* CacheList;
typedef struct memory_cache MemoryCache;
CacheList MemoryCacheCreate(int size, int count);
MemoryCache* MemoryCacheAlloc(CacheList clh, int size);
void MemoryCacheFree(CacheList clh, MemoryCache* mc_free);
MemoryCache* MemoryCacheGet(CacheList clh);