#ifndef DECODER_H
#define DECODER_H

#include "instruction.h"
#include "isa.h"
#include <stdint.h>
#include <stdio.h>
#include <ncurses.h>

#define MEMORY_SIZE 3072 // 3KB Total
#define INSTRUCTION_SIZE 1024 // 1KB pentru instructiuni
#define DATA_SIZE 1024 // 1KB pentru date
#define STACK_SIZE 1024 // 1KB pentru stiva

#define INSTRUCTION_START 0
#define INSTRUCTION_END 1023

#define DATA_START 1024
#define DATA_END 2047

#define STACK_START 2048
#define STACK_END 3071

#define NUM_REGISTERS 8

typedef struct {
    uint8_t ZF; // Zero Flag
    uint8_t SF; // Sign Flag
    uint8_t OF; // Overflow Flag
    uint8_t CF; // Carry Flag
} CPU_Flags;

// decodificarea unei instructiuni
Instruction decode_instruction(uint32_t instruction);

// decodificarea unui vector de instructiuni
void decode_instructions(uint32_t *instructions, int num_instructions, WINDOW *winL, WINDOW *winR);

#endif 
