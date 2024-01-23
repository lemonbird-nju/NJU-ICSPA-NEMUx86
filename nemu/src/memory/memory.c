#include "nemu.h"
#include "cpu/cpu.h"
#include "memory/memory.h"
#include "memory/mmu/cache.h"
#include "device/mm_io.h"
#include <memory.h>
#include <stdio.h>

uint8_t hw_mem[MEM_SIZE_B];

uint32_t hw_mem_read(paddr_t paddr, size_t len)
{
	uint32_t ret = 0;
	memcpy(&ret, hw_mem + paddr, len);
	return ret;
}

void hw_mem_write(paddr_t paddr, size_t len, uint32_t data)
{
	memcpy(hw_mem + paddr, &data, len);
}

uint32_t paddr_read(paddr_t paddr, size_t len)
{
	uint32_t ret = 0;
#ifdef HAS_DEVICE_VGA
    int ioNo = is_mmio(paddr);
    if (ioNo != -1)
    {
        ret = mmio_read(paddr, len, ioNo);
        return ret;
    }
#endif

#ifdef CACHE_ENABLED
    ret = cache_read(paddr, len);
#else
	ret = hw_mem_read(paddr, len);
#endif
	return ret;
}

void paddr_write(paddr_t paddr, size_t len, uint32_t data)
{
#ifdef HAS_DEVICE_VGA
    int ioNo = is_mmio(paddr);
    if (ioNo != -1)
    {
        mmio_write(paddr, len, data, ioNo);
        return;
    }
#endif

#ifdef CACHE_ENABLED
    cache_write(paddr, len, data);
#endif
	hw_mem_write(paddr, len, data);
}

uint32_t laddr_read(laddr_t laddr, size_t len)
{
    assert(len == 1 || len == 2 || len == 4);
    if (cpu.cr0.PE && cpu.cr0.PG)
    {
        uint32_t pg1 = laddr >> 12;
        uint32_t pg2 = (laddr + len - 1) >> 12;
        if (pg1 == pg2)
        {
            return paddr_read(page_translate(laddr), len);
        }
        else
        {
            size_t nlen = (pg2 << 12) - laddr;
            uint32_t low = paddr_read(page_translate(laddr), nlen);
            uint32_t high = paddr_read(page_translate(pg2 << 12), len - nlen);
            return (high << (nlen * 8)) + low;
        }
    } 
    
    return paddr_read(laddr, len);
}

void laddr_write(laddr_t laddr, size_t len, uint32_t data)
{
    assert(len == 1 || len == 2 || len == 4);
    if (cpu.cr0.PE && cpu.cr0.PG)
    {
        uint32_t pg1 = laddr >> 12;
        uint32_t pg2 = (laddr + len - 1) >> 12;
        if (pg1 == pg2)
        {
            paddr_write(page_translate(laddr), len, data);
            return;
        }
        else
        {
            size_t nlen = (pg2 << 12) - laddr;
            uint32_t high = data >> (nlen * 8);
            uint32_t low = data - (high << (nlen * 8));
            paddr_write(page_translate(laddr), nlen, low);
            paddr_write(page_translate(pg2 << 12), len - nlen, high);
            return;
        }
    }
    
	paddr_write(laddr, len, data);
}

uint32_t vaddr_read(vaddr_t vaddr, uint8_t sreg, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
#ifndef IA32_SEG
    return laddr_read(vaddr, len);
#else
    laddr_t laddr = vaddr;
	if (cpu.cr0.PE)
	{
	    laddr = segment_translate(vaddr, sreg);
	}
	return laddr_read(laddr, len);
#endif
}

void vaddr_write(vaddr_t vaddr, uint8_t sreg, size_t len, uint32_t data)
{
	assert(len == 1 || len == 2 || len == 4);
#ifndef IA32_SEG
    return laddr_write(vaddr, len, data);
#else
    laddr_t laddr = vaddr;
	if (cpu.cr0.PE)
	{
	    laddr = segment_translate(vaddr, sreg);
	}
	laddr_write(laddr, len, data);
#endif
}

void init_mem()
{
	// clear the memory on initiation
	memset(hw_mem, 0, MEM_SIZE_B);

#ifdef CACHE_ENABLED
    init_cache();
#endif

#ifdef TLB_ENABLED
	make_all_tlb();
	init_all_tlb();
#endif
}

uint32_t instr_fetch(vaddr_t vaddr, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
	return vaddr_read(vaddr, SREG_CS, len);
}

uint8_t *get_mem_addr()
{
	return hw_mem;
}
