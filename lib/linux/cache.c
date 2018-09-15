#include "linux/cache.h"

CacheList MemoryCacheCreate(int size, int count)
{
    CacheList clh = calloc(1, sizeof(CacheList_t));
    STAILQ_INIT(&clh->cache_head);

    clh->max_size = size;
    clh->max_count = count;
    clh->mem_size = 0;
    clh->cache_count = 0;
    return clh;
}

void MemoryCacheReset(CacheList clh)
{
    while (!STAILQ_EMPTY(&clh->cache_head))
    {
        MemoryCache* mc_remove = STAILQ_FIRST(&clh->cache_head);
        STAILQ_REMOVE_HEAD(&clh->cache_head, nodes);
        free(mc_remove->cache_addr);
        free(mc_remove);
    }

    clh->mem_size = 0;
    clh->cache_count = 0;
}

void MemoryCacheDestroy(CacheList clh)
{
    while (!STAILQ_EMPTY(&clh->cache_head))
    {
        MemoryCache* mc_remove = STAILQ_FIRST(&clh->cache_head);
        STAILQ_REMOVE_HEAD(&clh->cache_head, nodes);
        free(mc_remove);
    }
}

MemoryCache* MemoryCacheAlloc(CacheList clh)
{
    MemoryCache* mc_insert = malloc(sizeof(MemoryCache));
    mc_insert->cache_addr = calloc(1, clh->max_size);
    mc_insert->cache_size = clh->max_size;

    if (clh->cache_count > (clh->max_count - 1))
        return NULL;

    if (clh->mem_size > (clh->max_size * (clh->max_count - 1)))
        return NULL;

    STAILQ_INSERT_TAIL(&clh->cache_head, mc_insert, nodes);
    clh->mem_size += clh->max_size;
    clh->cache_count += 1;

    return mc_insert;
}

void MemoryCacheFree(CacheList clh, MemoryCache* mc_free)
{
    if (mc_free == NULL)
        return;

    if (clh->cache_count == 0)
        return;

    if (clh->mem_size == 0)
        return;

    STAILQ_REMOVE(&clh->cache_head, mc_free, memory_cache, nodes);
    clh->cache_count--;
    clh->mem_size -= clh->max_size;
    free(mc_free->cache_addr);
    free(mc_free);
}

MemoryCache* MemoryCacheGet(CacheList clh)
{
    return STAILQ_FIRST(&clh->cache_head);
}

MemoryCache* MemoryCacheNext(MemoryCache* cache)
{
    return STAILQ_NEXT(cache, nodes);
}

int MemoryCacheIsEmpty(CacheList clh)
{
    if (clh->mem_size && clh->cache_count)
        return 0;
    else
        return 1;
}

int MemoryCacheIsFull(CacheList clh)
{
    if (clh->cache_count == clh->max_count)
        return 1;
    else
        return 0;
}
