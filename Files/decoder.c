#include "decoder.h"
#include <stdio.h>

int32_t reg[NUM_REGISTERS] = {0};
uint32_t PC = 0;
uint32_t IR = 0;
int16_t STACK_P = STACK_END;
int32_t countJMS = 0;


CPU_Flags flags = {0};

void reset_flags() 
{
    flags.ZF = 0;
    flags.SF = 0;
    flags.OF = 0;
    flags.CF = 0;
}

void update_flags(int32_t result, int64_t real_res, int32_t op1, int32_t op2, int i) 
{
    reset_flags();
    flags.ZF = (result == 0) ? 1 : 0;
    flags.SF = (result < 0) ? 1 : 0;
    if(real_res > INT32_MAX)
    {
        flags.CF = 1;
    }

    if(i == 1) // adunare
    {
        if (((op1 > 0) && (op2 > 0) && (result < 0)) || ((op1 < 0) && (op2 < 0) && (result > 0))) 
        {
            flags.OF = 1;
        }
    }
    else if(i == 2) // scadere
    {   
        if (((op1 > 0) && (op2 < 0) && (result < 0)) || ((op1 < 0) && (op2 > 0) && (result > 0))) 
        {
            flags.OF = 1;
        }
    }
    else if(i == 3) // inmultire
    {
        if (result > INT32_MAX || result < INT32_MIN) 
        {
            flags.OF = 1;
        }
    }
    else if(i == 4) // impartire
    {
        if (op1 == INT32_MIN && op2 == -1) 
        {
            flags.OF = 1;
        }
    }
    // Alte update-uri daca e nevoie
}

int32_t memory[MEMORY_SIZE] = {0}; // Initializare memorie

// Convertim un index numeric intr-o adresa efectiva in memorie
static int32_t index_to_address(WINDOW *win, int32_t index) 
{
    if (index <= INSTRUCTION_END && index >= INSTRUCTION_START) 
    {
        // Nu permitem citirea in zona de instructiuni
        display_message(win, "Eroare: Nu se poate citi in zona de instructiuni la index: %d\n", index);
        exit(EXIT_FAILURE);
    } 
    else if (index >= DATA_START && index <= STACK_END) 
    {
        return index; // Zona date + date
    }
    else 
    {
        display_message(win, "Eroare: Index de memorie invalid: %d\n", index);
        exit(EXIT_FAILURE);
    }
}

// Citire din memorie 
int32_t read_memory_index(WINDOW* win, int32_t index) 
{
    int32_t address = index_to_address(win, index);
    return memory[address];
}

// Scriere in memorie
void write_memory_index(WINDOW *win, int32_t index, int32_t value) 
{
    int32_t address = index_to_address(win, index);
    if (address >= INSTRUCTION_START && address <= INSTRUCTION_END) 
    {
        // Nu permitem scrierea in zona de instructiuni
        display_message(win, "Eroare: Nu se poate scrie in zona de instructiuni la index: %d\n", index);
        exit(EXIT_FAILURE);
    }
    memory[address] = value;
}


void execute_instruction(Instruction instr, WINDOW *winL, WINDOW *winR) 
{
    switch(instr.opcode) 
    {
        case 0b0110011: // R-type (ADD, SUB, MUL, etc.)
        {
            switch(instr.funct3) 
            {
                case 0x0: 
                {
                    if (instr.funct7 == 0x00) // ADD sau MOV
                    {
                        int32_t op2 = reg[instr.rs2 - 8];
                        if(instr.rs1 != 0) // ADD
                        {
                            int32_t op1 = reg[instr.rs1 - 8];
                            int32_t result = op1 + op2;
                            reg[instr.rd - 8] = result;
                            update_flags(result, (int64_t)op1 + (int64_t)op2, op1, op2, 1);
                        }
                        else if(instr.rs1 == 0) // MOV
                        {
                            reg[instr.rd - 8] = op2;
                        }
                    } 
                    else if (instr.funct7 == 0x20)  // SUB
                    {
                        int32_t op1 = reg[instr.rs1 - 8];
                        int32_t op2 = reg[instr.rs2 - 8];
                        int32_t result = op1 - op2;
                        reg[instr.rd - 8] = result;
                        update_flags(result, (int64_t)op1 - (int64_t)op2, op1, op2, 2);
                    }
                    else if(instr.funct7 == 0x01) //MUL
                    {
                        int32_t op1 = reg[instr.rs1 - 8];
                        int32_t op2 = reg[instr.rs2 - 8];
                        int32_t result = op1 * op2;
                        reg[instr.rd - 8] = result;
                        update_flags(result, (int64_t)op1 * (int64_t)op2, op1, op2, 3);
                    }
                    break;
                }
                case 0x5: 
                {
                    if(instr.funct7 == 0x01) // UDV
                    {
                        int32_t op1 = reg[instr.rs1 - 8];
                        int32_t op2 = reg[instr.rs2 - 8];
                        if (op2 == 0) 
                        {
                            display_message(win, "Eroare: Împărțire la zero!\n");
                            PC = UINT32_MAX;
                            break;
                        }
                        int32_t result = op1 / op2;
                        reg[instr.rd - 8] = result;
                        update_flags(result, (int64_t)op1 / (int64_t)op2, op1, op2, 4);
                        break;
                    }
                    else if(instr.funct7 == 0x00) // LSR
                    {
                        int32_t op1 = reg[instr.rs1 - 8];
                        int32_t op2 = reg[instr.rs2 - 8];
                        int32_t result = op1 >> op2;
                        reg[instr.rd - 8] = result;
                        update_flags(result, (int64_t)op1 >> (int64_t)op2, op1, op2, 0);
                        break;
                    }
                }
                case 0x6:
                {
                    if(instr.funct7 == 0x01) // MOD
                    {
                        int32_t op1 = reg[instr.rs1 - 8];
                        int32_t op2 = reg[instr.rs2 - 8];
                        if (op2 == 0) 
                        {
                            display_message(win, "Eroare: Împărțire la zero pentru MOD!\n");
                            PC = UINT32_MAX;
                            break;
                        }
                        int32_t result = op1 % op2;
                        reg[instr.rd - 8] = result;
                        update_flags(result, (int64_t)op1 % (int64_t)op2, op1, op2, 0);
                        break;
                    }
                    else if(instr.funct7 == 0x00) // OR
                    {
                        int32_t op1 = reg[instr.rs1 - 8];
                        int32_t op2 = reg[instr.rs2 - 8];
                        int32_t result = op1 | op2;
                        reg[instr.rd - 8] = result;
                        update_flags(result, (int64_t)op1 | (int64_t)op2, op1, op2, 0);
                        break;
                    }
                }
                case 0x2: // CMP
                {
                    int32_t op1 = reg[instr.rs1 - 8];
                    int32_t op2 = reg[instr.rs2 - 8];
                    int32_t result = op1 - op2;
                    update_flags(result, (int64_t)op1 - (int64_t)op2, op1, op2, 0);
                    break;
                }
                case 0x7: // AND
                {
                    int32_t op1 = reg[instr.rs1 - 8];
                    int32_t op2 = reg[instr.rs2 - 8];
                    int32_t result = op1 & op2;
                    reg[instr.rd - 8] = result;
                    update_flags(result, (int64_t)op1 & (int64_t)op2, op1, op2, 0);
                    break;
                }
                case 0x4: // XOR
                {
                    int32_t op1 = reg[instr.rs1 - 8];
                    int32_t op2 = reg[instr.rs2 - 8];
                    int32_t result = op1 ^ op2;
                    reg[instr.rd - 8] = result;
                    update_flags(result, (int64_t)op1 ^ (int64_t)op2, op1, op2, 0);
                    break;
                }
                case 0x1: // LSL
                {
                    int32_t op1 = reg[instr.rs1 - 8];
                    int32_t op2 = reg[instr.rs2 - 8];
                    int32_t result = op1 << op2;
                    reg[instr.rd - 8] = result;
                    update_flags(result, (int64_t)op1 << (int64_t)op2, op1, op2, 0);
                    break;
                }
                default:
                {
                    display_message(win, "Eroare: Functie R-type necunoscuta!\n");
                    PC = UINT32_MAX;
                    break;
                }
            }
            break;
        }

        case 0b0010011: // I-type (ADDI, XORI, ORI, etc.)
        {
            switch(instr.funct3) 
            {
                case 0x0: 
                {
                    int32_t imm = instr.imm;
                    if(instr.rs1 == 0) // MOV Imm
                    {
                        reg[instr.rd - 8] = imm;
                    }
                    else // ADD Imm
                    {
                        int32_t op1 = reg[instr.rs1 - 8];
                        int32_t result = op1 + imm;
                        reg[instr.rd - 8] = result;
                        update_flags(result, (uint64_t)op1 + (uint64_t)imm, op1, imm, 1);
                    }
                    break;
                }
                case 0x3: // SUB Imm
                {
                    int32_t op1 = reg[instr.rs1 - 8];
                    int32_t imm = instr.imm;
                    int32_t result = op1 - imm;
                    reg[instr.rd - 8] = result;
                    update_flags(result, (uint64_t)op1 - (uint64_t)imm, op1, imm, 2);
                    break;
                }
                case 0x4: // XOR Imm
                {
                    int32_t op1 = reg[instr.rs1 - 8];
                    int32_t imm = instr.imm;
                    int32_t result = op1 ^ imm;
                    reg[instr.rd - 8] = result;
                    update_flags(result, (uint64_t)op1 ^ (uint64_t)imm, op1, imm, 0);
                    break;
                }
                case 0x6: // OR Imm
                {
                    int32_t op1 = reg[instr.rs1 - 8];
                    int32_t imm = instr.imm;
                    int32_t result = op1 | imm;
                    reg[instr.rd - 8] = result;
                    update_flags(result, (uint64_t)op1 | (uint64_t)imm, op1, imm, 0);
                    break;
                }
                case 0x7: // AND Imm
                {
                    int32_t op1 = reg[instr.rs1 - 8];
                    int32_t imm = instr.imm;
                    int32_t result = op1 & imm;
                    reg[instr.rd - 8] = result;
                    update_flags(result, (uint64_t)op1 & (uint64_t)imm, op1, imm, 0);
                    break;
                }
                case 0x1: // LSL Imm
                {
                    int32_t op1 = reg[instr.rs1 - 8];
                    int32_t imm = instr.imm;
                    int32_t result = op1 << imm;
                    reg[instr.rd - 8] = result;
                    update_flags(result, (int64_t)op1 << (int64_t)imm, op1, imm, 0);
                    break;
                }
                case 0x5: // LSR Imm
                {
                    int32_t op1 = reg[instr.rs1 - 8];
                    int32_t imm = instr.imm;
                    int32_t result = op1 >> imm;
                    reg[instr.rd - 8] = result;
                    update_flags(result, (int64_t)op1 >> (int64_t)imm, op1, imm, 0);
                    break;
                }
                case 0x2: // CMP Imm
                {
                    int32_t op1 = reg[instr.rs1 - 8];
                    int32_t imm = instr.imm;
                    int32_t result = op1 - imm;
                    update_flags(result, (int64_t)op1 - (int64_t)imm, op1, imm, 0);
                    break;
                }
                default:
                {
                    display_message(win, "Eroare: Functie I-type necunoscuta!\n");
                    PC = UINT32_MAX;
                    break;
                }
            }
            break;
        }

        case 0b1100011: // B-type (Branch)
        {
            switch(instr.funct3)
            {
                case 0x0:  // BEQ / BRZ / BRA
                {
                    if (instr.deriv == 0) // BEQ
                    {
                        if (reg[instr.rs1 - 8] == reg[instr.rs2 - 8])
                        {
                            PC = instr.imm; // Absolute jump
                        }
                    }
                    else if (instr.deriv == 1) // BRZ
                    {
                        if (flags.ZF == 1)
                        {
                            PC = instr.imm; // Absolute jump
                        }
                    }
                    else if (instr.deriv == 2) // BRA
                    {
                        PC = instr.imm; // Absolute jump
                    }
                    else if (instr.deriv == 4) // JMS
                    {
                        memory[countJMS] = PC + 1;
                        countJMS++;
                        if(countJMS > INSTRUCTION_END)
                        {
                            display_message(win, "Eroare: S-au depasit numarul de jump-uri succesive permise!\n");
                            PC = UINT32_MAX;
                            break;
                        }
                        PC = instr.imm; // Absolute jump
                    }
                    break;
                }

                case 0x1: // BNE / BVS
                {
                    if (instr.deriv == 0) // BNE
                    {
                        if (reg[instr.rs1 - 8] != reg[instr.rs2 - 8])
                        {
                            PC = instr.imm; // Absolute jump
                        }
                    }
                    else if (instr.deriv == 2) // BVS
                    {
                        if (flags.OF == 1)
                        {
                            PC = instr.imm; // Absolute jump
                        }
                    }
                    break;
                }

                case 0x4: // BLT / BMI / BLE
                {
                    if (instr.deriv == 0) // BLT
                    {
                        if (reg[instr.rs1 - 8] < reg[instr.rs2 - 8])
                        {
                            PC = instr.imm; // Absolute jump
                        }
                    }
                    else if (instr.deriv == 1) // BMI
                    {
                        if (flags.SF == 1)
                        {
                            PC = instr.imm; // Absolute jump
                        }
                    }
                    else if (instr.deriv == 2) // BLE
                    {
                        if (reg[instr.rs1 - 8] <= reg[instr.rs2 - 8])
                        {
                            PC = instr.imm; // Absolute jump
                        }
                    }
                    break;
                }

                case 0x5: // BGE / BRP / BGT / BCS / BPL
                {
                    if (instr.deriv == 0) // BGE
                    {
                        if (reg[instr.rs1 - 8] >= reg[instr.rs2 - 8])
                        {
                            PC = instr.imm; // Absolute jump
                        }
                    }
                    else if (instr.deriv == 1) // BRP
                    {
                        if (flags.ZF == 0 && flags.SF == 0)
                        {
                            PC = instr.imm; // Absolute jump
                        }
                    }
                    else if (instr.deriv == 2) // BGT
                    {
                        if (reg[instr.rs1 - 8] > reg[instr.rs2 - 8])
                        {
                            PC = instr.imm; // Absolute jump
                        }
                    }
                    else if (instr.deriv == 3) // BCS
                    {
                        if (flags.CF == 1)
                        {
                            PC = instr.imm; // Absolute jump
                        }
                    }
                    else if (instr.deriv == 4) // BPL
                    {
                        if (flags.SF == 0)
                        {
                            PC = instr.imm; // Absolute jump
                        }
                    }
                    break;
                }

                case 0x6: // BLTU
                {
                    if ((uint32_t)reg[instr.rs1 - 8] < (uint32_t)reg[instr.rs2 - 8])
                    {
                        PC = instr.imm; // Absolute jump
                    }
                    break;
                }

                case 0x7: // BGEU
                {
                    if ((uint32_t)reg[instr.rs1 - 8] >= (uint32_t)reg[instr.rs2 - 8])
                    {
                        PC = instr.imm; // Absolute jump
                    }
                    break;
                }

                default:
                {
                    display_message(win, "Eroare: Functie B-type necunoscuta!\n");
                    PC = UINT32_MAX;
                    break;
                }
            }
            break;
        }

        // instructiunile LOAD si STORE aici
        case 0b0100011: // STORE
        {   
            if(instr.funct3 == 0x1) // PSH
            {
                if (STACK_P < STACK_START) 
                {
                    display_message(win, "Eroare: Stack Overflow la PUSH!\n");
                    PC = UINT32_MAX;
                    break;
                }

                int32_t reg_value = reg[instr.rd - 8]; 
                write_memory_index(win, STACK_P, reg_value); 
                STACK_P--;
            }
            else
            {
                int32_t imm = instr.imm;
                int32_t poz = imm + DATA_START;
                if(poz > DATA_END)
                {
                    display_message(win, "Eroare: Ati depasit zona de scriere de date la instructiunea %d!\n", PC);
                    PC = UINT32_MAX;
                    break;
                }
                if(instr.rs1 == 0) // STA
                {
                    write_memory_index(win, poz, reg[instr.rd - 8]);
                    PC = UINT32_MAX;    
                    break;
                }
                else // STR
                {
                    int32_t op1 = reg[instr.rs1 - 8];
                    poz = poz + op1;
                    if(poz > DATA_END)
                    {
                        display_message(win, "Eroare: Ati depasit zona de scriere de date la instructiunea %d!\n", PC);
                        PC = UINT32_MAX;
                        break;
                    }
                    write_memory_index(win, poz, reg[instr.rd - 8]);
                }
            }
            break;
        }

        case 0b0000011: // LOAD
        {   
            if(instr.funct3 == 0x1) // POP
            {
                if (STACK_P >= STACK_END) 
                {
                    display_message(win, "Eroare: Stack Underflow la POP!\n");
                    PC = UINT32_MAX;
                    break;
                }

                STACK_P++;
                reg[instr.rd - 8] = read_memory_index(win, STACK_P);
            }
            else
            {
                int32_t imm = instr.imm;
                int32_t poz = imm + DATA_START;
                if(poz > DATA_END)
                {
                    display_message(win, "Eroare: Ati depasit zona de scriere de date la instructiunea %d!\n", PC);
                    PC = UINT32_MAX;
                    break;
                }
                if(instr.rs1 == 0) // LDA
                {
                    reg[instr.rd - 8] = read_memory_index(win, poz);
                    break;
                }
                else // LDR
                {
                    int32_t op1 = reg[instr.rs1 - 8];
                    poz = op1 + poz;
                    if(poz > DATA_END)
                    {
                        display_message(win, "Eroare: Ati depasit zona de scriere de date la instructiunea %d!\n", PC);
                        PC = UINT32_MAX;
                        break;
                    }
                    reg[instr.rd - 8] = read_memory_index(win, poz);
                }
            }
            break;
        }

        case 0b1110011: // HALT sau RET
        {
            if(instr.funct3 == 0x0) // HALT
            {
                display_message(win, "HALT: Executia s-a oprit.\n");
                PC = UINT32_MAX;
                break;
            }
            else if(instr.funct3 == 0x1) //RET
            {
                countJMS--;
                if(countJMS >= INSTRUCTION_START)
                {
                    PC = memory[countJMS];
                }
                else
                {
                    display_message(win, "Eroare: RET nu se poate intoarce.\n");
                    PC = UINT32_MAX;
                }
            }
            break;
        }

        default:
        {
            display_message(win, "Eroare: Opcode necunoscut.\n");            
            PC = UINT32_MAX;
            break;
        }
    }
}

// Decodificarea unei instructiuni
Instruction decode_instruction(uint32_t instruction) 
{
    Instruction instr;
    instr.raw = instruction;
    instr.opcode = instruction & 0x7F;  // Extragem opcode-ul

    // Identificam tipul instructiunii
    switch (instr.opcode) 
    {
        case 0b0110011: // R-type
        {
            instr.type = 'R';
            instr.rd = (instruction >> 7) & 0x1F;
            instr.funct3 = (instruction >> 12) & 0x7;
            instr.rs1 = (instruction >> 15) & 0x1F;
            instr.rs2 = (instruction >> 20) & 0x1F;
            instr.funct7 = (instruction >> 25) & 0x7F;
            instr.imm = 0; // Nu exista imediat in instrucțiunile R
            instr.deriv = 0; // Nu exista derivat in instrucțiunile R
            break;
        }

        case 0b0010011: // I-type
        case 0b0000011: // I-type (Load)
        case 0b0100011: // I-type (Store)        
        case 0b1110011: // I-type (System Instructions)
        {
            instr.type = 'I';
            instr.rd = (instruction >> 7) & 0x1F;
            instr.funct3 = (instruction >> 12) & 0x7;
            instr.rs1 = (instruction >> 15) & 0x1F;
            instr.imm = (int16_t)(instruction >> 20); // Semnat, bitii 20-31
            instr.funct7 = 0; // Nu exista funct7 in I-type
            instr.deriv = 0; // Nu exista derivat in instrucțiunile I
            break;
        }

        case 0b1100011: // B-type
        {
            instr.type = 'B';
            instr.funct3 = (instruction >> 12) & 0x7;
            instr.rs1 = (instruction >> 7) & 0x1F;
            instr.rs2 = (instruction >> 19) & 0x1F;
            instr.deriv = (instruction >> 15) & 0xF;
            instr.imm = (int16_t)(instruction >> 24); // Offset semnat
            instr.rd = 0; // Nu exista rd in instrucțiunile B
            instr.funct7 = 0; // Nu exista funct7 in B-type
            break;
        }

        default:
            instr.type = '?'; // Tip necunoscut
            instr.rd = instr.rs1 = instr.rs2 = instr.funct3 = instr.funct7 = instr.deriv = 0;
            instr.imm = 0;
            break;
    }

    return instr;
}

// Decodificarea unui vector de instructiuni
void decode_instructions(uint32_t *instructions, int num_instructions, WINDOW *winL, WINDOW *winR) 
{
    if (!instructions || num_instructions <= 0) 
    {
        display_message(win, "Eroare: Vectorul de instructiuni este invalid!\n");
        return;
    }

    while(PC < num_instructions) 
    {
        Instruction instr = decode_instruction(instructions[PC]);
        if(instr.type != '?')
        {
            // display_message(win, "Cod Instructiune %d = 0x%08X!\n", PC, instr);
            uint32_t vechiPC = PC;
            execute_instruction(instr, winL, winR);
            if(PC == vechiPC)
                PC++;

            // Afisam registrii si flagurile
            werase(win);
            box(win, 0, 0);
            mvwprintw(win, 2, 1, "PC: %d", PC);
            mvwprintw(win, 3, 1, "Registri:");
            for (int i = 0; i < NUM_REGISTERS; i++) 
            {
                mvwprintw(win, 4 + i, 1, "R%d: %d", i, reg[i]);
            }
            mvwprintw(win, 12, 1, "Flaguri:");
            mvwprintw(win, 13, 1, "ZF: %d", flags.ZF);
            mvwprintw(win, 14, 1, "SF: %d", flags.SF);
            mvwprintw(win, 15, 1, "OF: %d", flags.OF);
            mvwprintw(win, 16, 1, "CF: %d", flags.CF);
            wrefresh(win);

            int ch;
            do 
            {
                ch = wgetch(winR);
            } while (ch != '\n'); // ENTER pentru a continua  

        }
        else
        {
            display_message(win, "Eroare: Instructiune necunoscuta la adresa PC=%d!\n", PC);
            return;
        }
    }
}
