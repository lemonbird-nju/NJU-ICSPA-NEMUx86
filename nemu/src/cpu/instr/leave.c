#include "cpu/instr.h"

make_instr_func(leave) {
    OPERAND esp, ebp, rm;
    
    esp.data_size = ebp.data_size = 32;
    esp.type = ebp.type = OPR_REG;
    esp.addr = 0x4;
    ebp.addr = 0x5;
    
    operand_read(&ebp);
    esp.val = ebp.val;
    
    rm.data_size = 32;
    rm.type = OPR_MEM;
    rm.sreg = SREG_SS;
    rm.addr = esp.val;
    
    operand_read(&rm);
    esp.val += 4;
    ebp.val = rm.val;
    
    operand_write(&esp);
    operand_write(&ebp);
    
    print_asm_0("leave", "", 1);
    return 1;
}
