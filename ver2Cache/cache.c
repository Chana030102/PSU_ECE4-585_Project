/*
 * Define necessary functions to work with the cache
 */
#include "cache.h"

/******* Globals *******/
// Keep track of transactions
int total_accesses = 0;
int writes = 0;   
int reads = 0;
int hits = 0;
int misses = 0;
int evictions = 0;
int writebacks = 0;


/******* Functions *******/
/*
 * Set all LRU, dirty, and valid bits as 0
 */
void init_cache(struct cache_set* sys_cache)
{
    for(int i=0;i<cache_ways;i++)
    {
        sys_cache->valid[i] = 0;
        sys_cache->dirty[i] = 0;
        sys_cache->LRU[i] = 0;
    }
}

/*
 * Search for corresponding tag from trace.
 * Upon hit, return corresponding way
 * Upon miss, return -1 
 */
int cache_compare_tag(unsigned int tag, struct cache_set* req_set)
{
   for(int i=0;i<cache_ways;i++)
   {
      if(req_set->tag[i] == tag)
      {
          if(req_set->valid[i]==1)
          {
              hits++;
              return i;
          }
      }
   }
   misses++;
   return -1;
}

/*
 * Look for invalid cache lines we can replace.
 * Return corresponding way when invalid found.
 * If all lines valid, then return -1.
 */
int cache_compare_valid(struct cache_set* req_set)
{
    for(int i=0;i<cache_ways;i++)
    {
        if(req_set->valid[i]==0)
            return i;
    }
    return -1;
}

/* 
 * Compare LRU bits and evict a cache line
 * If all LRU bits set to 1, reset all to 0
 */
int cache_evict(struct cache_set* req_set)
{
    int to_evict = -1;
    // Check for LRU bit = 0
    for(int i=0;i<cache_ways;i++)
    {
        if(req_set->LRU[i]==0)
        {
            to_evict=i;
            break;
        }
    }

    // If all LRU bits were 1, set all to 0
    // and tell cache to evict the first way
    if(to_evict == -1)
    {
        for(int i=0;i<cache_ways;i++)
            req_set->LRU[i] = 0;
        to_evict = 0;
    }
    
    // If to_evict way has dirty bit set,
    // then increment writebacks
    if(req_set->dirty[to_evict]==1)
        writebacks++;
    evictions++;
    return to_evict;

}

/* 
 * Perform Cache Operation
 * Check for hit or miss and then perform the
 * specified operation.
 *
 * Valid and LRU bits will be set on every operatoin
 * Only the Write operation will set the dirty bit
 */
int cache_op(unsigned int tag, struct cache_set* req_set)
{
    int way = cache_compare_tag(tag,req_set); // determine cache hit/miss
    
    if(way < 0) // Cache miss
    {
       way = cache_compare_valid(req_set);
       if(way < 0)
           way = cache_evict(req_set);

       if(r_w_bit) // Cache Write on miss
       {
           req_set->valid[way] = 1;
           req_set->dirty[way] = 1;
           req_set->LRU[way] = 1;
           req_set->tag[way] = tag;
           writes++;
       }
       else // Cache Read on miss
       {
           req_set->valid[way] = 1;
           req_set->dirty[way] = 0;
           req_set->LRU[way] = 1;
           req_set->tag[way] = tag;
           reads++;
       }

    }
    else  // Cache hit
    {
       if(r_w_bit) // Cache Write on hit
       {
           req_set->dirty[way] = 1;
           req_set->LRU[way] = 1;
           writes++;
       }
       else // Cache Read on hit
       {
           req_set->LRU[way] = 1;
           reads++;
       }
    }
    return 1;
}

/* 
 * Display the performance statistics of
 * the cache with the applied trace
 */
void cache_stats(void)
{
    float hit_ratio = (hits/(float)total_accesses)*100;
    float miss_ratio = (misses/(float)total_accesses)*100;

    printf("========== Cache Performance Results ==========\n\n");
    printf("Total Cache Accesses: %d\n",total_accesses);
    printf("Number of Cache Reads: %d\n",reads);
    printf("Number of Cache Writes: %d\n",writes);
    printf("Number of Cache Hits: %d\n",hits);
    printf("Number of Cache Misses: %d\n",misses);
    printf("Cache Hit Ratio: %2.2f%%\n",hit_ratio);
    printf("Cache Miss Ratio: %2.2f%%\n",miss_ratio);
    printf("Number of Evictions: %d\n",evictions);
    printf("Number of Writebacks: %d\n\n",writebacks);
    printf("===============================================\n\n");
}
