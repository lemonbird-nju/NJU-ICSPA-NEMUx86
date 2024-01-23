#include "cpu/instr.h"

static void instr_execute_1op()
{
    OPERAND stk;
    stk.data_size = 32;
    stk.addr = cpu.esp;
    stk.type = OPR_MEM;
    stk.sreg = SREG_SS;
    operand_read(&stk);
    
    opr_src.sreg = SREG_DS;
    opr_src.val = stk.val;
    
    operand_write(&opr_src);
    cpu.esp += 4;
}

make_instr_impl_1op(pop, r, v)

make_instr_func(popa)
{
    uint32_t temp;
    uint32_t *reg[] = {&cpu.edi, &cpu.esi, &cpu.ebp, &temp, &cpu.ebx, &cpu.edx, &cpu.ecx, &cpu.eax};
    
    for (int i = 0; i < 8; i++)
    {
        opr_src.data_size = 32;
        opr_src.type = OPR_MEM;
        opr_src.sreg = SREG_SS;
        opr_src.addr = cpu.esp;
        operand_read(&opr_src);
        *(reg[i]) = opr_src.val;
        cpu.esp += 4;
    }
    
    return 1;
}
