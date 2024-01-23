#include "cpu/instr.h"

make_instr_func(ret_near_imm16) {
    OPERAND esp, rm, imm;
    
    esp.data_size = 32;
    esp.type = OPR_REG;
    esp.addr = 0x4;
    
    operand_read(&esp);
    
    rm.data_size = 32;
    rm.type = OPR_MEM;
    rm.sreg = SREG_SS;
    rm.addr = esp.val;
    
    imm.data_size = 16;
    imm.type = OPR_IMM;
    imm.addr = eip + 1;
    
    operand_read(&rm);
    operand_read(&imm);
    
    print_asm_1("ret", "", 3, &imm);
    
    esp.val += 4 + imm.val;
    cpu.eip = rm.val;
    operand_write(&esp);
    
    return 0;
}

make_instr_func(ret_near) {
    OPERAND esp, rm;
    
    esp.data_size = 32;
    esp.type = OPR_REG;
    esp.addr = 0x4;
    
    operand_read(&esp);
    
    rm.data_size = 32;
    rm.type = OPR_MEM;
    rm.sreg = SREG_SS;
    rm.addr = esp.val;
    
    operand_read(&rm);
    
    print_asm_0("ret", "", 1);
    
    esp.val += 4;
    cpu.eip = rm.val;
    operand_write(&esp);
    
    return 0;
}
