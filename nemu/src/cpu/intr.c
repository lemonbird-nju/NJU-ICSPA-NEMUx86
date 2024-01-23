#include "cpu/intr.h"
#include "cpu/instr.h"
#include "memory/memory.h"

void raise_intr(uint8_t intr_no)
{
#ifdef IA32_INTR
    // Push EFLAGS, CS, and EIP
    OPERAND opr_src;
    opr_src.data_size = 32;
    opr_src.type = OPR_MEM;
    opr_src.sreg = SREG_SS;
    
    cpu.esp -= 4;
    opr_src.val = cpu.eflags.val;
    opr_src.addr = cpu.esp;
    operand_write(&opr_src);
    
    cpu.esp -= 4;
    opr_src.val = cpu.cs.val;
    opr_src.addr = cpu.esp;
    operand_write(&opr_src);
    
    cpu.esp -= 4;
    opr_src.val = cpu.eip;
    opr_src.addr = cpu.esp;
    operand_write(&opr_src);
    
    // Find the IDT entry using 'intr_no'
    GateDesc gateDesc;
    gateDesc.val[0] = laddr_read(cpu.idtr.base + intr_no * 8, 4);
    gateDesc.val[1] = laddr_read(cpu.idtr.base + intr_no * 8 + 4, 4);

    // Clear IF if it is an interrupt
    if (gateDesc.type == 0xE)
        cpu.eflags.IF = 0;
    
    // Set CS:EIP to the entry of the interrupt handler, need to reload CS with load_sreg()
    uint32_t addr = (gateDesc.offset_31_16 << 16) + gateDesc.offset_15_0;
    cpu.cs.val = gateDesc.selector;
    load_sreg(SREG_CS);
    cpu.eip = addr;
#endif
}

void raise_sw_intr(uint8_t intr_no)
{
	// return address is the next instruction
	cpu.eip += 2;
	raise_intr(intr_no);
}
