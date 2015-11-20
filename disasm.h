/*
 * disasm.h
 *
 *  Created on: Nov 20, 2015
 *      Author: koen
 */

#ifndef DISASM_H_
#define DISASM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>

#define VERSION_INFO "v2.0"
#define NUMBER_OPCODES 151

/* Exceptions for cycle counting */
#define CYCLES_CROSS_PAGE_ADDS_ONE      (1 << 0)
#define CYCLES_BRANCH_TAKEN_ADDS_ONE    (1 << 1)

/** Some compilers don't have EOK in errno.h */
#ifndef EOK
#define EOK 0
#endif

/* Helper macros for disassemble() function */
#define DUMP_FORMAT (hex_output ? "%-16s%-16s;" : "%-8s%-16s;")
#define HIGH_PART(val) (((val) >> 8) & 0xFFu)
#define LOW_PART(val) ((val) & 0xFFu)
#define LOAD_WORD(buffer, current_pc) ((uint16_t)buffer[(current_pc) + 1] | (((uint16_t)buffer[(current_pc) + 2]) << 8))

class disasm
{
private:
	/* The 6502's 13 addressing modes */
	typedef enum {
	    IMMED = 0, /* Immediate */
	    ABSOL, /* Absolute */
	    ZEROP, /* Zero Page */
	    IMPLI, /* Implied */
	    INDIA, /* Indirect Absolute */
	    ABSIX, /* Absolute indexed with X */
	    ABSIY, /* Absolute indexed with Y */
	    ZEPIX, /* Zero page indexed with X */
	    ZEPIY, /* Zero page indexed with Y */
	    INDIN, /* Indexed indirect (with X) */
	    ININD, /* Indirect indexed (with Y) */
	    RELAT, /* Relative */
	    ACCUM /* Accumulator */
	} addressing_mode_e;

	typedef struct opcode_s {
	    uint8_t number; /* Number of the opcode */
	    const char *mnemonic; /* Index in the name table */
	    addressing_mode_e addressing; /* Addressing mode */
	    unsigned int cycles; /* Number of cycles */
	    unsigned int cycles_exceptions; /* Mask of cycle-counting exceptions */
	} opcode_t;

public:
	uint16_t getDisassembly(char *output, uint8_t *buffer, int hex_output, uint16_t pc);
};


#endif /* DISASM_H_ */
