#include "linux/cache.h"

STAILQ_HEAD(cache_list_head, memory_cache) cache_head =
    STAILQ_HEAD_INITIALIZER(cache_head);

struct cache_list_head* p_cache_head;
int    max_size = 0;
int    mem_size = 0;
int    cache_count = 0;
int    max_count = 0;

CacheList MemoryCacheCreate(int size, int count)
{
    STAILQ_INIT(&cache_head);
    p_cache_head = &cache_head;

    max_size = size;
    max_count = count;
    mem_size = 0;
    cache_count = 0;

    return p_cache_head;
}

void MemoryCacheReset(CacheList clh)
{
    while (!STAILQ_EMPTY(clh))
    {
        MemoryCache* mc_remove = STAILQ_FIRST(clh);
        STAILQ_REMOVE_HEAD(clh, nodes);
        free(mc_remove->cache_addr);
        free(mc_remove);
    }

    mem_size = 0;
    cache_count = 0;
}

void MemoryCacheDestroy(CacheList clh)
{
    while (!STAILQ_EMPTY(clh))
    {
        MemoryCache* mc_remove = STAILQ_FIRST(clh);
        STAILQ_REMOVE_HEAD(clh, nodes);
        free(mc_remove);
    }
}

MemoryCache* MemoryCacheAlloc(CacheList clh)
{
    MemoryCache* mc_insert = malloc(sizeof(MemoryCache));
    mc_insert->cache_addr = calloc(1, max_size);
    mc_insert->cache_size = max_size;

    if (cache_count > (max_count - 1))
        return NULL;

    if (mem_size > (max_size * (max_count - 1)))
        return NULL;

    STAILQ_INSERT_TAIL(&cache_head, mc_insert, nodes);
    mem_size += max_size;
    cache_count += 1;

    return mc_insert;
}

void MemoryCacheFree(CacheList clh, MemoryCache* mc_free)
{
    if (mc_free == NULL)
        return;

    if (cache_count == 0)
        return;

    if (mem_size == 0)
        return;

    STAILQ_REMOVE(&cache_head, mc_free, memory_cache, nodes);
    cache_count--;
    mem_size -= max_size;
    free(mc_free->cache_addr);
    free(mc_free);
}

MemoryCache* MemoryCacheGet(CacheList clh)
{
    return STAILQ_FIRST(&cache_head);
}

MemoryCache* MemoryCacheNext(MemoryCache* cache)
{
    return STAILQ_NEXT(cache, nodes);
}

int MemoryCacheIsEmpty(CacheList clh)
{
    if (mem_size && cache_count)
        return 0;
    else
        return 1;
}

int MemoryCacheIsFull(CacheList clh)
{
    if (cache_count == max_count)
        return 1;
    else
        return 0;
}
