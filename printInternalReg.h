

/* This file contiains the prototypes and 
   constants needed to use the routines 
   defined in printRoutines.c
*/

#ifndef PRINTINTERNALREG
#define PRINTINTERNALREG

#include <stdint.h>

#define PRINTERROR -1
#define PRINTSUCCESS 0

typedef unsigned char nibble;

void printReg(uint64_t PC, nibble icode, nibble ifun, 
	     int regsValid, nibble rA, nibble rB, 
             int  valCValid, uint64_t valC,
	     uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3,  
	     uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7,  
	     int64_t valP, char * instr);

#endif /* PRINTROUTINES */
