#include "cpu/instr.h"

make_instr_func(call_near) {
    OPERAND rel, esp, loc;
    
    rel.data_size = data_size;
    rel.type = OPR_IMM;
    rel.addr = eip + 1;
    
    esp.data_size = 32;
    esp.type = OPR_REG;
    esp.addr = 0x4;
    
    operand_read(&esp);
    
    loc.data_size = 32;
    loc.type = OPR_MEM;
    loc.val = eip + 1 + data_size / 8;
    
    esp.val -= 4;
    loc.addr = esp.val;
    
    operand_write(&esp);
    operand_write(&loc);
    print_asm_1("call", "", 1 + data_size / 8, &rel);
    
    operand_read(&rel);
    cpu.eip += rel.val;
    
    return 1 + data_size / 8;
}

make_instr_func(call_near_indirect) {
    OPERAND rm, esp, loc;
    
    rm.data_size = data_size;
    int len = 1;
    len += modrm_rm(eip + 1, &rm);
    
    esp.data_size = 32;
    esp.type = OPR_REG;
    esp.addr = 0x4;
    
    operand_read(&esp);
    
    loc.data_size = 32;
    loc.type = OPR_MEM;
    loc.val = eip + len;
    
    esp.val -= 4;
    loc.addr = esp.val;
    
    operand_write(&esp);
    operand_write(&loc);
    print_asm_1("call", "", len, &rm);
    
    operand_read(&rm);
    cpu.eip = rm.val;
    
    return 0;
}
