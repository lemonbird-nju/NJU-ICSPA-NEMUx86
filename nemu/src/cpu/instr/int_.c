#include "cpu/intr.h"
#include "cpu/instr.h"

/*
Special note for `int': please use the instruction name `int_' instead of `int'.
*/

make_instr_func(int_)
{
    OPERAND imm;
    
    imm.type = OPR_IMM;
    imm.sreg = SREG_CS;
    imm.addr = eip + 1;
    imm.data_size = 8;
    
    operand_read(&imm);
    
    uint8_t intr_no = imm.val;
    raise_sw_intr(intr_no);
    
    return 0;
}
