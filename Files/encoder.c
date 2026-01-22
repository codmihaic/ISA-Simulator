#include "encoder.h"

uint32_t encode_B_type(uint8_t opcode, uint8_t funct3, uint8_t rs1, uint8_t rs2, uint8_t deriv, int16_t imm)
{
    uint32_t instruction = 0;
    instruction |= (imm & 0x1FFF) << 24;     // imm[12:0]= Biti 24-31
    instruction |= (deriv & 0xF) << 15;      // deriv= Biti 15-18
    instruction |= (funct3 & 0x7) << 12;      // funct3= Biti 12-14
    instruction |= (rs2 & 0x1F) << 19;       // rs2= Biti 19-23
    instruction |= (rs1 & 0x1F) << 7;       // rs1= Biti 7-11
    instruction |= (opcode & 0x7F);          // opcode= Biti 0-6
    return instruction;
}

uint32_t encode_BR_instruction(const char *operation, const char *imm_label, int rs1, int rs2, const int impl) 
{
    int imm = getEtichetaAddress(imm_label) - 1; // instructiunea de salt = offset
    uint8_t deriv = 0; // 0 = fundamentala, >0 = derivata

    if ((rs2 < 0 || rs2 > 8) || (rs1 < 0 || rs1 > 8) || (imm < 0 || imm > 255))
    {
        printf("Eroare: Offset invalid pentru instrucțiunea de tip Branch!\n");
        return UINT32_MAX;
    }

    if(impl != 0)
    {
        rs1 += 8;
        rs2 += 8;
    }

    // fundamentale:
    if (strcasecmp(operation, "bra") == 0) 
    {
        return encode_B_type(0b1100011, 0x0, 0, 0, 2, imm); 
    }
    else if (strcasecmp(operation, "beq") == 0) 
    {
        return encode_B_type(0b1100011, 0x0, rs1, rs2, 0, imm); 
    }
    else if (strcasecmp(operation, "bne") == 0) 
    {
        return encode_B_type(0b1100011, 0x1, rs1, rs2, 0, imm); 
    }
    else if (strcasecmp(operation, "blt") == 0) 
    {
        return encode_B_type(0b1100011, 0x4, rs1, rs2, 0, imm); 
    }
    else if (strcasecmp(operation, "bge") == 0) 
    {
        return encode_B_type(0b1100011, 0x5, rs1, rs2, 0, imm); 
    }
    else if (strcasecmp(operation, "bltu") == 0) 
    {
        deriv = 4;
        return encode_B_type(0b1100011, 0x6, rs1, rs2, 0, imm); 
    }
    else if (strcasecmp(operation, "bgeu") == 0) 
    {
        deriv = 4;
        return encode_B_type(0b1100011, 0x7, rs1, rs2, 0, imm); 
    }

    // derivate:
    if (strcasecmp(operation, "brz") == 0) 
    {
        deriv = 1;
        return encode_B_type(0b1100011, 0x0, 0, 0, deriv, imm); // BRZ din BEQ
    }
    else if (strcasecmp(operation, "brp") == 0) 
    {
        deriv = 1;
        return encode_B_type(0b1100011, 0x5, 0, 0, deriv, imm); // BRP din BGE
    }
    else if (strcasecmp(operation, "bmi") == 0) 
    {
        deriv = 1;
        return encode_B_type(0b1100011, 0x4, 0, 0, deriv, imm); // BMI din BLT
    }
    else if (strcasecmp(operation, "bgt") == 0) 
    {
        deriv = 2;
        return encode_B_type(0b1100011, 0x5, rs1, rs2, deriv, imm); // BGT din BGE
    }
    else if (strcasecmp(operation, "ble") == 0) 
    {
        deriv = 2;
        return encode_B_type(0b1100011, 0x4, rs1, rs2, deriv, imm); // BLE din BLT
    }
    else if (strcasecmp(operation, "bvs") == 0) 
    {
        deriv = 2;
        return encode_B_type(0b1100011, 0x1, 0, 0, deriv, imm); // BVS din BNE
    }
    else if (strcasecmp(operation, "bcs") == 0) 
    {
        deriv = 3;
        return encode_B_type(0b1100011, 0x5, 0, 0, deriv, imm); // BCS din BGE
    }
    else if (strcasecmp(operation, "bpl") == 0) 
    {
        deriv = 4;
        return encode_B_type(0b1100011, 0x5, 0, 0, deriv, imm); // BPL din BGE
    }
    else if (strcasecmp(operation, "jms") == 0) 
    {
        deriv = 4;
        return encode_B_type(0b1100011, 0x0, 0, 0, deriv, imm); // JMS din BEQ
    }

    printf("Eroare: Instrucțiunea Branch '%s' nu este validă!\n", operation);
    return UINT32_MAX;
}

uint32_t encode_I_type(uint8_t opcode, uint8_t rd, uint8_t funct3, uint8_t rs1, int16_t imm) 
{
    uint32_t instruction = 0; 
    instruction |= (imm & 0xFFF) << 20;  // imm[11:0]= Biti 20-31
    instruction |= (rs1 & 0x1F) << 15;  // rs1= Biti 15-19
    instruction |= (funct3 & 0x7) << 12; // funct3= Biti 12-14
    instruction |= (rd & 0x1F) << 7;    // rd= Biti 7-11
    instruction |= (opcode & 0x7F);     // opcode= Biti 0-6
    return instruction;
}

uint32_t encode_I_instruction(const char *operation, int rd, int rs1, const int imm, const int impl)
{
    if ((rd < 0 || rd > 8) || (rs1 < 0 || rs1 > 8) || (imm < -2048 || imm > 2047)) 
    {
        printf("Eroare: Registru sau valoare imediata invalida!\n");
        return UINT32_MAX;
    }

    // de siguranta pentru cazul in care avem rs1 implicit rd
    rd += 8;
    if(impl != 0)
        rs1 += 8;
    else
        rs1 = rd;

    if (strcasecmp(operation, "add") == 0) 
    {
        return encode_I_type(0b0010011, rd, 0x0, rs1, imm);
    }
    else if (strcasecmp(operation, "sub") == 0) 
    {
        return encode_I_type(0b0010011, rd, 0x3, rs1, imm);
    }
    else if (strcasecmp(operation, "xor") == 0) 
    {
        return encode_I_type(0b0010011, rd, 0x4, rs1, imm);
    } 
    else if (strcasecmp(operation, "or") == 0) 
    {
        return encode_I_type(0b0010011, rd, 0x6, rs1, imm);
    } 
    else if (strcasecmp(operation, "and") == 0) 
    {
        return encode_I_type(0b0010011, rd, 0x7, rs1, imm);
    } 
    else if (strcasecmp(operation, "lsl") == 0) 
    {
        return encode_I_type(0b0010011, rd, 0x1, rs1, imm);
    } 
    else if (strcasecmp(operation, "lsr") == 0) 
    {
        return encode_I_type(0b0010011, rd, 0x5, rs1, imm);
    } 
    else if (strcasecmp(operation, "asr") == 0) 
    {
        return encode_I_type(0b0010011, rd, 0x5, rs1, imm | (0x20 << 5));
    } 
    else if (strcasecmp(operation, "cmp") == 0) 
    {
        // implementat cu SLTI (Set Less Than Immediate)
        return encode_I_type(0b0010011, rd, 0x2, rs1, imm);
    }
    else if(strcasecmp(operation, "mov") == 0)
    {
        // MOV va fi implementat ca ADD cu rs1 = 0
        return encode_I_type(0b0010011, rd, 0x0, 0, imm); 
    }
    else if (strcasecmp(operation, "lda") == 0) 
    {
        return encode_I_type(0b0000011, rd, 0x2, 0, imm);
    }
    else if (strcasecmp(operation, "ldr") == 0) 
    {
        return encode_I_type(0b0000011, rd, 0x2, rs1, imm);
    }
    else if (strcasecmp(operation, "str") == 0) 
    {
        return encode_I_type(0b0100011, rd, 0x2, rs1, imm);
    }
    else if (strcasecmp(operation, "sta") == 0) 
    {
        return encode_I_type(0b0100011, rd, 0x2, 0, imm);
    }
    else if (strcasecmp(operation, "psh") == 0) 
    {
        return encode_I_type(0b0100011, rd, 0x1, 0, 0);
    }
    else if (strcasecmp(operation, "pop") == 0) 
    {
        return encode_I_type(0b0000011, rd, 0x1, 0, 0);
    }
    else if (strcasecmp(operation, "ret") == 0)
    {
        // ret poate fi interpretată ca ECALL 
        return encode_I_type(0b1110011, 0, 0x1, 0, 0);
    }

    printf("Eroare: Instructiunea I-type '%s' nu este valida!\n", operation);
    return UINT32_MAX;
}

uint32_t encode_R_type(uint8_t opcode, uint8_t rd, uint8_t funct3, uint8_t rs1, uint8_t rs2, uint8_t funct7) 
{
    uint32_t instruction = 0;
    instruction |= (funct7 & 0x7F) << 25;   // funct7= Biti 25-31
    instruction |= (rs2 & 0x1F) << 20;     // rs2= Biti 20-24
    instruction |= (rs1 & 0x1F) << 15;     // rs1= Biti 15-19
    instruction |= (funct3 & 0x7) << 12;   // funct3= Biti 12-14
    instruction |= (rd & 0x1F) << 7;       // rd= Biti 7-11
    instruction |= (opcode & 0x7F);        // opcode= Biti 0-6
    return instruction;
}

uint32_t encode_R_instruction(const char *operation, int rd, int rs1, int rs2, const int impl)
{
    if((rd < 0 || rd > 8) || (rs1 < 0 || rs1 > 8) || (rs2 < 0 || rs2 > 8))
    {
        printf("Eroare: Registru invalid");
        return UINT32_MAX;
    }

    rd += 8;
    rs2 += 8;
    // de siguranta pentru cazul in care avem rs1 implicit rd
    if(impl != 0)
        rs1 += 8;
    else
        rs1 = rd;

    if(strcasecmp(operation, "add") == 0)
    {
        return encode_R_type(0b0110011, rd, 0x0, rs1, rs2, 0x00);
    }
    else if(strcasecmp(operation, "sub") == 0)
    {
        return encode_R_type(0b0110011, rd, 0x0, rs1, rs2, 0x20); 
    }
    else if(strcasecmp(operation, "mul") == 0)
    {
        return encode_R_type(0b0110011, rd, 0x0, rs1, rs2, 0x01);
    }
    else if(strcasecmp(operation, "udv") == 0)
    {
        return encode_R_type(0b0110011, rd, 0x5, rs1, rs2, 0x01);
    }
    else if(strcasecmp(operation, "mod") == 0)
    {
        return encode_R_type(0b0110011, rd, 0x6, rs1, rs2, 0x01);
    }
    else if(strcasecmp(operation, "cmp") == 0)
    {
        // folosim codul de la SLT (Set Less Than)
        return encode_R_type(0b0110011, rd, 0x2, rs1, rs2, 0x00); 
    }
    else if(strcasecmp(operation, "mov") == 0)
    {
        // MOV va fi implementat ca ADD cu rs1 = 0
        return encode_R_type(0b0110011, rd, 0x0, 0, rs2, 0x00); 
    }
    else if(strcasecmp(operation, "and") == 0)
    {
        return encode_R_type(0b0110011, rd, 0x7, rs1, rs2, 0x00);
    }
    else if(strcasecmp(operation, "or") == 0)
    {
        return encode_R_type(0b0110011, rd, 0x6, rs1, rs2, 0x00);
    }
    else if(strcasecmp(operation, "xor") == 0)
    {
        return encode_R_type(0b0110011, rd, 0x4, rs1, rs2, 0x00);
    }
    else if(strcasecmp(operation, "lsl") == 0)
    {
        return encode_R_type(0b0110011, rd, 0x1, rs1, rs2, 0x00);
    }
    else if(strcasecmp(operation, "lsr") == 0)
    {
        return encode_R_type(0b0110011, rd, 0x5, rs1, rs2, 0x00);
    }
    else if(strcasecmp(operation, "hlt") == 0)
    {
        // halt poate fi interpretată ca ECALL 
        return encode_I_type(0b1110011, 0, 0x0, 0, 0);
    }

    printf("Eroare: Instructiunea R-type '%s' nu este valida!\n", operation);
    return UINT32_MAX; // eroare
}
