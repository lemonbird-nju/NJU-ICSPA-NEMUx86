#include "cpu/instr.h"

static void instr_execute_1op()
{
    OPERAND stk;
    
    operand_read(&opr_src);
    cpu.esp -= 4;
    
    stk.val = sign_ext(opr_src.val, opr_src.data_size);
    stk.data_size = 32;
    stk.type = OPR_MEM;
    stk.sreg = SREG_SS;
    stk.addr = cpu.esp;
    
    operand_write(&stk);
}

make_instr_impl_1op(push, r, v)
make_instr_impl_1op(push, i, v)
make_instr_impl_1op(push, i, b)
make_instr_impl_1op(push, rm, v)

make_instr_func(pusha)
{
    uint32_t temp;
    temp = cpu.esp;
    
    uint32_t reg[] = {cpu.eax, cpu.ecx, cpu.edx, cpu.ebx, temp, cpu.ebp, cpu.esi, cpu.edi};
    
    for (int i = 0; i < 8; i++)
    {
        cpu.esp -= 4;
        opr_src.val = reg[i];
        opr_src.data_size = 32;
        opr_src.type = OPR_MEM;
        opr_src.sreg = SREG_SS;
        opr_src.addr = cpu.esp;
        operand_write(&opr_src);
    }
    
    return 1;
}
