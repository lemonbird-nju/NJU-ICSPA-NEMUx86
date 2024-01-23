#include "memory/mmu/cache.h"

#include <stdlib.h>
#include <time.h>

#ifdef CACHE_ENABLED

struct CacheLine cache[1024];
extern uint8_t hw_mem[];

// init the cache
void init_cache()
{
    for (int i = 0; i < 1024; i++)
        cache[i].valid = 0;
}

// write data to cache
void cache_write(paddr_t paddr, size_t len, uint32_t data)
{
	// implement me in PA 3-1
	uint32_t offset, setno, tag, lineno;
	offset = paddr & 0x3F;
	setno = (paddr >> 6) & 0x7F;
	tag = paddr >> 13;
	
	size_t mlen = 0;
	
	for (lineno = setno * 8; lineno < (setno + 1) * 8; lineno++)
	{
        if (cache[lineno].valid && cache[lineno].tag == tag)
        {
            mlen = len < 64 - offset ? len : 64 - offset;
            memcpy(cache[lineno].data + offset, &data, mlen);
            if (mlen < len)
                cache_write(paddr + mlen, len - mlen, data >> (mlen * 8));
        }
	}
}

// read data from cache
uint32_t cache_read(paddr_t paddr, size_t len)
{
	// implement me in PA 3-1
	uint32_t offset, setno, tag, lineno;
	offset = paddr & 0x3F;
	setno = (paddr >> 6) & 0x7F;
	tag = paddr >> 13;
	
	uint32_t ret = 0, remain;
	size_t mlen = 0;
	
	for (lineno = setno * 8; lineno < (setno + 1) * 8; lineno++)
	{
        if (cache[lineno].valid && cache[lineno].tag == tag)
        {
            mlen = len < 64 - offset ? len : 64 - offset;
            memcpy(&ret, cache[lineno].data + offset, mlen);
            if (mlen < len)
            {
                remain = cache_read(paddr + mlen, len - mlen);
                ret += remain << (mlen * 8);
            }
            return ret;
        }
	}
	
	uint32_t lineno_sub;
	paddr_t paddr_block = paddr & 0xFFFFFFC0;  // address of the start of this block
	
	for (lineno_sub = setno * 8; lineno_sub < (setno + 1) * 8; lineno_sub++)
        if (!cache[lineno_sub].valid)
            break;
	if (lineno_sub == (setno + 1) * 8)
	{
        srand(time(NULL));
        lineno_sub = setno * 8 + rand() % 8;
	}
	
	memcpy(cache[lineno_sub].data, hw_mem + paddr_block, 64);
	cache[lineno_sub].valid = 1;
	cache[lineno_sub].tag = tag;
	
	mlen = len < 64 - offset ? len : 64 - offset;
    memcpy(&ret, cache[lineno_sub].data + offset, mlen);
    if (mlen < len)
    {
        remain = cache_read(paddr + mlen, len - mlen);
        ret += remain << (mlen * 8);
    }
	
	return ret;
}

#endif
