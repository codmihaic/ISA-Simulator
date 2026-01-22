#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>

// Structura pentru o instructiune decodificata
typedef struct 
{
    uint32_t raw;    // Instructiunea bruta
    uint8_t opcode;  // Codul operatiei
    uint8_t funct3;  // Funcție secundara pentru identificare (R/I/B)
    uint8_t funct7;  // Funcție secundara pentru instrucțiuni de tip R
    uint8_t rd;      // Registru destinatie
    uint8_t rs1;     // Primul registru sursa
    uint8_t rs2;     // Al doilea registru sursa
    int16_t imm;     // Valoare imediata
    uint8_t deriv;   // Indicator derivata (doar pentru instrucțiuni B)
    char type;       // Tipul instructiunii (R, I, B)
} Instruction;

#endif 
