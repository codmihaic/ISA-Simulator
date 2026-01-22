#ifndef ENCODER_H
#define ENCODER_H

#include "parse_code.h"
#include "isa.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

// functii encoding
uint32_t encode_R_type(uint8_t opcode, uint8_t rd, uint8_t funct3, uint8_t rs1, uint8_t rs2, uint8_t funct7);
uint32_t encode_I_type(uint8_t opcode, uint8_t rd, uint8_t funct3, uint8_t rs1, int16_t imm);
uint32_t encode_B_type(uint8_t opcode, uint8_t funct3, uint8_t rs1, uint8_t rs2, uint8_t deriv, int16_t imm);

uint32_t encode_R_instruction(const char *operation, int rd, int rs1, int rs2, int is_mov);
uint32_t encode_I_instruction(const char *operation, int rd, int rs1, const int imm, const int impl);
uint32_t encode_BR_instruction(const char *operation, const char *imm_label, int rs1, int rs2, const int impl);

#endif // ENCODER_H
