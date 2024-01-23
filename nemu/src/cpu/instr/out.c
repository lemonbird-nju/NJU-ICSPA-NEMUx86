#include "cpu/instr.h"
#include "device/port_io.h"

make_instr_func(out_b)
{
    OPERAND al, dx;
    
	al.data_size = 8;
	al.type = OPR_REG;
	al.addr = REG_AL;
	operand_read(&al);
	
	dx.data_size = 16;
	dx.type = OPR_REG;
	dx.addr = REG_DX;
	operand_read(&dx);
	
	pio_write(dx.val, 1, al.val);
	
	return 1;
}

make_instr_func(out_v)
{
    OPERAND eax, dx;
    
	eax.data_size = data_size;
	eax.type = OPR_REG;
	eax.addr = REG_EAX;
	operand_read(&eax);
	
	dx.data_size = 16;
	dx.type = OPR_REG;
	dx.addr = REG_DX;
	operand_read(&dx);
	
	pio_write(dx.val, data_size / 8, eax.val);
	
	return 1;
}
