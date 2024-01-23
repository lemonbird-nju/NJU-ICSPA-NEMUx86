#include "cpu/cpu.h"
#include "memory/memory.h"

// translate from linear address to physical address
paddr_t page_translate(laddr_t laddr)
{
#ifndef TLB_ENABLED
	uint32_t dir = (laddr >> 22) & 0x3FF;
	uint32_t page = (laddr >> 12) & 0x3FF;
	uint32_t offset = laddr & 0xFFF;
	
	PDE pde;
	paddr_t pde_paddr = (cpu.cr3.PDBR << 12) + 4 * dir;
	pde.val = paddr_read(pde_paddr, 4);
	assert(pde.present);
	
	PTE pte;
	paddr_t pte_paddr = (pde.page_frame << 12) + 4 * page;
	pte.val = paddr_read(pte_paddr, 4);
	assert(pte.present);
	
	paddr_t paddr = (pte.page_frame << 12) + offset;
	return paddr;
#else
	return tlb_read(laddr) | (laddr & PAGE_MASK);
#endif
}
