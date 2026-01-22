#ifndef PARSE_CODE_H
#define PARSE_CODE_H

#include "encoder.h"
#include "isa.h"
#include "decoder.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>

#define MAX_ET 255
#define MAX_LINII 255

typedef struct l {
    int adresa;
    int opcode;
    int regs[3];
    int use_imm;
    int use_dat;
} linie;

typedef struct ea {
    int adresa;
    char *nume;
} eticheta;

// variabile globale importante
extern uint32_t *instructiuni;
extern int iCount;
extern WINDOW *win, *win1;

// functii pentru etichete
void addEticheta(const char *nume, int address);
int getEtichetaAddress(const char *nume);
int inArray(const char *key, char *array[], int size);
int etichetaValida(const char *label);

// functii pentru linii si instructiuni
// void prelucrando(char *linie);
char* prelucrandoEtichete(char *linie);
void procesandoInstructiuni(char *linie, const int lAcum);
void parse_lines(char **lines, int num_lines, WINDOW *winL, WINDOW *winR);
#endif 
