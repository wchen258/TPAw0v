#ifndef TRACE_H_  
#define TRACE_H_

#include <stdint.h>
#include <stdio.h>

#include "handlers.h"

#define Async                   0b00000000
#define TraceInfo               0b00000001 		
#define LongAddress0            0b10011101 	
#define LongAddress1            0b10011110 
#define LongAddress2            0b10011011 
#define LongAddress3            0b10011010 
#define ShortAddr0              0b10010110 
#define ShortAddr1              0b10010101 
#define AddrWithContext0        0b10000010 
#define AddrWithContext1        0b10000011 
#define AddrWithContext2        0b10000101 
#define AddrWithContext3        0b10000110 
#define TimeStamp0              0b00000010     
#define TimeStamp1              0b00000011  
#define Atom10                  0b11110111 
#define Atom11                  0b11110110
#define Atom20                  0b11011000
#define Atom21                  0b11011001
#define Atom22                  0b11011010
#define Atom23                  0b11011011
#define Atom40                  0b11011100
#define Atom41                  0b11011101
#define Atom42                  0b11011110
#define Atom43                  0b11011111
#define Atom50                  0b11010111 
#define Atom51                  0b11010110 
#define Atom52                  0b11010101 
#define Atom53                  0b11110101 
#define ExactMatch0             0b10010000 
#define ExactMatch1             0b10010001
#define ExactMatch2             0b10010010
#define Exce                    0b00000110 
#define ExceReturn              0b00000111
#define Context0                0b10000000
#define Context1                0b10000001
#define FunctionReturn          0b00000101
#define TraceOn                 0b00000100
#define Resync                  0b00001000
#define CCF10                   0b00001111
#define CCF11                   0b00001110
#define CCF20                   0b00001101 
#define CCF21                   0b00001100

#define CC_THRESHOLD 4

typedef struct address_reg {
    uint64_t address;
    uint8_t is;
} address_reg_t;

void trace_loop(void);

void handle_async(void);
void handle_resync(void);
void handle_traceinfo(void);
void handle_longaddress(uint8_t);
void handle_shortaddress(uint8_t);
void handle_exactmatch(uint8_t);
void handle_addrwithcontext(uint8_t);
void handle_context(uint8_t);
void handle_timestamp(uint8_t);
void handle_atom1(uint8_t);
void handle_atom2(uint8_t);
void handle_atom3(uint8_t);
void handle_atom4(uint8_t);
void handle_atom5(uint8_t);
void handle_atom6(uint8_t);
void handle_event(uint8_t);
void handle_exception(void);
void handle_exceptionreturn(void);
void handle_functionreturn(void);
void handle_traceon(void);
void handle_ccf1(uint8_t);
void handle_ccf2(uint8_t);
void handle_ccf3(uint8_t);

#endif // TRACE_H_
