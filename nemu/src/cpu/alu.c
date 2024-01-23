#include "cpu/cpu.h"

void set_CF_add(uint32_t result, uint32_t src, size_t data_size) {
	result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
	src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
	cpu.eflags.CF = result < src; 
}

void set_CF_adc(uint32_t result, uint32_t src, size_t data_size) {
	result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
	src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
	if (cpu.eflags.CF) cpu.eflags.CF = result <= src;
	else cpu.eflags.CF = result < src; 
}

void set_CF_sub(uint32_t dest, uint32_t src, size_t data_size) {
    dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
	src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
	cpu.eflags.CF = dest < src;
}

void set_CF_sbb(uint32_t dest, uint32_t src, size_t data_size) {
    dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
	src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
	if (cpu.eflags.CF) cpu.eflags.CF = dest <= src;
	else cpu.eflags.CF = dest < src;
}

void set_PF(uint32_t result) {
    int num = 1;
    for (int i = 1; i <= 8; i++) {
        if (result % 2) num++;
        result /= 2;
    }
    cpu.eflags.PF = num % 2;
}

void set_ZF(uint32_t result, size_t data_size) {
	result = result & (0xFFFFFFFF >> (32 - data_size));
	cpu.eflags.ZF = (result == 0);
}

void set_SF(uint32_t result, size_t data_size) {
	result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
	cpu.eflags.SF = sign(result);
}

void set_OF_add(uint32_t result, uint32_t src, uint32_t dest, size_t data_size) {
	switch(data_size) {
		case 8:
			result = sign_ext(result & 0xFF, 8);
			src = sign_ext(src & 0xFF, 8);
			dest = sign_ext(dest & 0xFF, 8);
			break;
		case 16:
			result = sign_ext(result & 0xFFFF, 16);
			src = sign_ext(src & 0xFFFF, 16);
			dest = sign_ext(dest & 0xFFFF, 16);
			break;
		default: break;
	}
	
	if(sign(src) == sign(dest)) {
		if(sign(src) != sign(result))
			cpu.eflags.OF = 1;
		else
			cpu.eflags.OF = 0;
	}
	else {
		cpu.eflags.OF = 0;
	}
}

void set_OF_sub(uint32_t result, uint32_t src, uint32_t dest, size_t data_size) {
    switch(data_size) {
		case 8:
			result = sign_ext(result & 0xFF, 8);
			src = sign_ext(src & 0xFF, 8);
			dest = sign_ext(dest & 0xFF, 8);
			break;
		case 16:
			result = sign_ext(result & 0xFFFF, 16);
			src = sign_ext(src & 0xFFFF, 16);
			dest = sign_ext(dest & 0xFFFF, 16);
			break;
		default: break;
	}
	
	if(sign(src) != sign(dest)) {
		if(sign(src) == sign(result))
			cpu.eflags.OF = 1;
		else
			cpu.eflags.OF = 0;
	}
	else {
		cpu.eflags.OF = 0;
	}
}


uint32_t alu_add(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_add(src, dest, data_size);
#else
	uint32_t res = 0;
	res = dest + src;
	set_CF_add(res, src, data_size);
	set_PF(res);
	set_ZF(res, data_size);
	set_SF(res, data_size);
	set_OF_add(res, src, dest, data_size);
	return res & (0xFFFFFFFF >> (32 - data_size));
#endif
}

uint32_t alu_adc(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_adc(src, dest, data_size);
#else
	uint32_t res = 0;
	res = dest + src + cpu.eflags.CF;
	set_CF_adc(res, src, data_size);
	set_PF(res);
	set_ZF(res, data_size);
	set_SF(res, data_size);
	set_OF_add(res, src, dest, data_size);
	return res & (0xFFFFFFFF >> (32 - data_size));
#endif
}

uint32_t alu_sub(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sub(src, dest, data_size);
#else
	uint32_t res = 0;
	res = dest - src;
	set_CF_sub(dest, src, data_size);
	set_PF(res);
	set_ZF(res, data_size);
	set_SF(res, data_size);
	set_OF_sub(res, src, dest, data_size);
	return res & (0xFFFFFFFF >> (32 - data_size));
#endif
}

uint32_t alu_sbb(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sbb(src, dest, data_size);
#else
	uint32_t res = 0;
	res = dest - src - cpu.eflags.CF;
	set_CF_sbb(dest, src, data_size);
	set_PF(res);
	set_ZF(res, data_size);
	set_SF(res, data_size);
	set_OF_sub(res, src, dest, data_size);
	return res & (0xFFFFFFFF >> (32 - data_size));
#endif
}

uint64_t alu_mul(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_mul(src, dest, data_size);
#else
	uint64_t res = 0;
	res = (uint64_t)dest * (uint64_t)src;
	if (res & (0xFFFFFFFFFFFFFFFF << data_size)) {
	    cpu.eflags.CF = 1;
	    cpu.eflags.OF = 1;
	}
	else {
	    cpu.eflags.CF = 0;
	    cpu.eflags.OF = 0;
	}
	return res;
#endif
}

int64_t alu_imul(int32_t src, int32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_imul(src, dest, data_size);
#else
	int64_t res = 0;
	res = (int64_t)dest * (int64_t)src;
	return res;
#endif
}

// need to implement alu_mod before testing
uint32_t alu_div(uint64_t src, uint64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_div(src, dest, data_size);
#else
	assert(src != 0);
	uint32_t res = 0;
	res = dest / src;
	return res;
#endif
}

// need to implement alu_imod before testing
int32_t alu_idiv(int64_t src, int64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_idiv(src, dest, data_size);
#else
	assert(src != 0);
	int32_t res = 0;
	res = dest / src;
	return res;
#endif
}

uint32_t alu_mod(uint64_t src, uint64_t dest)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_mod(src, dest);
#else
    assert(src != 0);
	uint32_t res = 0;
	res = dest % src;
	return res;
#endif
}

int32_t alu_imod(int64_t src, int64_t dest)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_imod(src, dest);
#else
	assert(src != 0);
	int32_t res = 0;
	res = dest % src;
	return res;
#endif
}

uint32_t alu_and(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_and(src, dest, data_size);
#else
	uint32_t res = 0;
	res = src & dest;
	cpu.eflags.CF = 0;
	set_PF(res);
	set_ZF(res, data_size);
	set_SF(res, data_size);
	cpu.eflags.OF = 0;
	
	return res & (0xFFFFFFFF >> (32 - data_size));
#endif
}

uint32_t alu_xor(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_xor(src, dest, data_size);
#else
	uint32_t res = 0;
	res = src ^ dest;
	cpu.eflags.CF = 0;
	set_PF(res);
	set_ZF(res, data_size);
	set_SF(res, data_size);
	cpu.eflags.OF = 0;
	
	return res & (0xFFFFFFFF >> (32 - data_size));
#endif
}

uint32_t alu_or(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_or(src, dest, data_size);
#else
	uint32_t res = 0;
	res = src | dest;
	cpu.eflags.CF = 0;
	set_PF(res);
	set_ZF(res, data_size);
	set_SF(res, data_size);
	cpu.eflags.OF = 0;
	
	return res & (0xFFFFFFFF >> (32 - data_size));
#endif
}

uint32_t alu_shl(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_shl(src, dest, data_size);
#else
	uint32_t res = dest, count = src;
	while (count) {
	    cpu.eflags.CF = res >> (data_size - 1);
	    res = res * 2;
	    count--;
	}
	set_PF(res);
	set_ZF(res, data_size);
	set_SF(res, data_size);
	if (src == 1) {
	    cpu.eflags.OF = (res >> (data_size - 1)) != cpu.eflags.CF;
	}
	return res & (0xFFFFFFFF >> (32 - data_size));
#endif
}

uint32_t alu_shr(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_shr(src, dest, data_size);
#else
	uint32_t res = dest, count = src;
	while (count) {
	    cpu.eflags.CF = res % 2;
	    res = res / 2;
	    count--;
	}
	res = res & (0xFFFFFFFF >> (32 - data_size + src));
	set_PF(res);
	set_ZF(res, data_size);
	set_SF(res, data_size);
	if (src == 1) {
	    cpu.eflags.OF = (res >> (data_size - 1));
	}
	return res;
#endif
}

uint32_t alu_sar(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sar(src, dest, data_size);
#else
	uint32_t res = dest, count = src;
	uint32_t sign = (dest >> (data_size - 1)) & 0x00000001;
	while (count) {
	    cpu.eflags.CF = res % 2;
	    res = res / 2;
	    count--;
	}
	if (sign) {
	    res = (0xFFFFFFFF << (data_size - src)) | res;
	    res = res & (0xFFFFFFFF >> (32 - data_size));
	}
	else {
	    res = res & (0xFFFFFFFF >> (32 - data_size + src));
	}
	set_PF(res);
	set_ZF(res, data_size);
	set_SF(res, data_size);
	if (src == 1) {
	    cpu.eflags.OF = 0;
	}
	return res;
#endif
}

uint32_t alu_sal(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sal(src, dest, data_size);
#else
	uint32_t res = dest, count = src;
	while (count) {
	    cpu.eflags.CF = res >> (data_size - 1);
	    res = res * 2;
	    count--;
	}
	set_PF(res);
	set_ZF(res, data_size);
	set_SF(res, data_size);
	if (src == 1) {
	    cpu.eflags.OF = (res >> (data_size - 1)) != cpu.eflags.CF;
	}
	return res & (0xFFFFFFFF >> (32 - data_size));
#endif
}
