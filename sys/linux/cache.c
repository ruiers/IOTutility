#include "linux/cache.h"

STAILQ_HEAD(cache_list_head, memory_cache) cache_head =
    STAILQ_HEAD_INITIALIZER(cache_head);

struct cache_list_head* p_cache_head;
char   *mem_pool = NULL;
int    max_size = 0;
int    mem_size = 0;
int    cache_count = 0;
int    max_count = 0;

CacheList MemoryCacheCreate(int size, int count)
{
    STAILQ_INIT(&cache_head);
    p_cache_head = &cache_head;
    mem_pool = calloc(count + 1 , size);
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
        free(mc_remove);
    }

    memset(mem_pool, 0x0, max_size);
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

    free(mem_pool);
}

MemoryCache* MemoryCacheAlloc(CacheList clh)
{
    MemoryCache* mc_insert = malloc(sizeof(MemoryCache));
    mc_insert->cache_addr = mem_pool + cache_count * max_size;
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
    free(mc_free);
}

MemoryCache* MemoryCacheGet(CacheList clh)
{
    return STAILQ_FIRST(&cache_head);
}

