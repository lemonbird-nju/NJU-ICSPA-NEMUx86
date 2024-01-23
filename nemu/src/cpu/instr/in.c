#include "cpu/instr.h"
#include "device/port_io.h"

make_instr_func(in_b)
{
    OPERAND al, dx;
    
	al.data_size = 8;
	al.type = OPR_REG;
	al.addr = REG_AL;
	
	dx.data_size = 16;
	dx.type = OPR_REG;
	dx.addr = REG_DX;
	operand_read(&dx);
	
	al.val = pio_read(dx.val, 1);
	operand_write(&al);
	
	return 1;
}

make_instr_func(in_v)
{
    OPERAND eax, dx;
    
	eax.data_size = data_size;
	eax.type = OPR_REG;
	eax.addr = REG_EAX;
	
	dx.data_size = 16;
	dx.type = OPR_REG;
	dx.addr = REG_DX;
	operand_read(&dx);
	
	eax.val = pio_read(dx.val, data_size / 8);
	operand_write(&eax);
	
	return 1;
}