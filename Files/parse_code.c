#include "parse_code.h"

char *ops[] = {"hlt", "mod", "add", "sub", "cmp", "mov", "and", "or", "xor", "udv", "mul", "lsr", "lsl", "str", "sta", "ldr", "lda", "ret", "psh", "pop"};
char *branches[] = {"bra", "beq", "bne", "blt", "bge", "brz", "brp", "bmi", "bgt", "ble", "bvs", "bcs", "bpl", "bltu", "bgeu", "jms"};

eticheta etichete[MAX_ET];
int etichete_count = 0;
linie linii[MAX_LINII];
int linii_count = 0;
int lCurenta = 0;
uint32_t *instructiuni = NULL;
int iCount = 0; // contor pentru liniile valide
WINDOW *win, *win1;

void addEticheta(const char *nume, int address) 
{
    for (int i = 0; i < etichete_count; i++) 
    {
        if (strcmp(etichete[i].nume, nume) == 0) 
        {
            handle_error(win, "Eroare: Eticheta '%s' este deja definita!\n", nume);
            end_ncurses(win1, win);
        }
    }
    etichete[etichete_count].nume = strdup(nume);
    etichete[etichete_count].adresa = address;
    etichete_count++;
}

int getEtichetaAddress(const char *nume) 
{
    for (int i = 0; i < etichete_count; i++) 
    {
        if (strcmp(etichete[i].nume, nume) == 0) {
            return etichete[i].adresa;
        }
    }
    return -1;
}

int inArray(const char *key, char *array[], int size) 
{
    for (int i = 0; i < size; i++) 
    {
        if (strcasecmp(key, array[i]) == 0) 
        {
            return 1;
        }
    }
    return 0;
}

int etichetaValida(const char *label) 
{
    if (!isalpha(label[0])) 
    { 
        return 0; 
    }
    for (int i = 1; label[i] != '\0'; i++) 
    {
        if (!isalnum(label[i])) 
        { 
            return 0;
        }
    }

    if(inArray(label, ops, sizeof(ops)/sizeof(ops[0])) == 1 || inArray(label, branches, sizeof(branches)/sizeof(branches[0])) == 1)
    {    
        return 0;
    }
    return 1; 
}

char* prelucrandoEtichete(char *linie) 
{
    char *formated_line = calloc(strlen(linie) + 1, sizeof(char));   
    int idx = 0;
    int aux = 0;

    for (int i = 0; i < strlen(linie); i++) 
    {
        if (linie[i] == ' ' || linie[i] == '\t') 
        {
            if (aux == 1) 
            {
                formated_line[idx++] = ' '; 
                aux = 0;
            }
            continue;
        } 
        else if (linie[i] == ',') 
        {
            if(isalnum(linie[i + 1]))
            {
                formated_line[idx++] = ' '; 
            }
            continue;
        }
        else 
        {
            formated_line[idx++] = linie[i];
            aux = 1;
        }
    }

    if (idx > 0 && formated_line[idx - 1] == ' ') 
    {
        formated_line[--idx] = '\0';
    } 
    else 
    {
        formated_line[idx] = '\0';
    }

    // daca e comentariu
    if (formated_line[0] == '/' && formated_line[1] == '/') 
    {
        lCurenta--;
        free(formated_line);
        return NULL;
    }
    lCurenta++;

    char *tmp = strdup(formated_line); 
    if (!tmp) 
    {
        free(formated_line);
        handle_error(win, "Eroare: Alocare eșuată pentru tmp!\n");
        end_ncurses(win1, win);
    }
    char *token = strtok(tmp, " ");
    char **elemente = NULL;
    int nEl = 0;

    while (token != NULL) 
    {
        char **temp = realloc(elemente, (nEl + 1) * sizeof(char *));
        if (!temp) 
        {
            free(tmp);
            free(formated_line);
            handle_error(win, "Eroare: Alocare eșuată pentru elemente!\n");
            end_ncurses(win1, win);
        }
        elemente = temp;

        elemente[nEl] = strdup(token);
        if (!elemente[nEl]) 
        {
            free(tmp);
            free(formated_line);
            handle_error(win, "Eroare: Alocare eșuată pentru elemente[%d]!\n", nEl);
            end_ncurses(win1, win);
        }
        nEl++;
        token = strtok(NULL, " ");
    }
    free(tmp);

    if(nEl > 5)
    {
        free(formated_line);
        handle_error(win, "Eroare la linia %s: Prea multe elemente pe linie!\n", linie);
        end_ncurses(win1, win);
    }

    if (nEl > 1 && etichetaValida(elemente[0])) 
    {
        addEticheta(elemente[0], lCurenta);
        free(elemente[0]);
        for (int i = 0; i < nEl - 1; i++) 
        {
            elemente[i] = elemente[i + 1];
        }
        nEl--;
    }

    
    char *result = calloc(strlen(formated_line) + 1, sizeof(char));
    for (int i = 0; i < nEl; i++) 
    {
        char *pos = strchr(elemente[i], '#'); // Caută '#'
        if (pos != NULL && pos != elemente[i]) 
        {
            char temp[256];
            snprintf(temp, sizeof(temp), "%.*s %s", (int)(pos - elemente[i]), elemente[i], pos);
            strcat(result, temp);
        } 
        else 
        {
            strcat(result, elemente[i]); 
        }

        if(i < nEl - 1)
        {
            strcat(result, " ");
        }
    }

    for (int i = 0; i < nEl; i++) 
    {
        free(elemente[i]);
    }
    free(elemente);
    free(formated_line);
    return result;
}

void procesandoInstructiuni(char *linie, const int lAcum)
{
    char *tmp = strdup(linie);
    if (!tmp) 
    {
        handle_error(win, "Eroare: Alocare eșuată pentru tmp!\n");
        end_ncurses(win1, win);
    }

    char *token = strtok(tmp, " ");
    char **elemente = NULL;
    int nEl = 0;

    while (token != NULL) 
    {
        char **temp = realloc(elemente, (nEl + 1) * sizeof(char *));
        if (!temp) 
        {
            free(tmp);
            handle_error(win, "Eroare: Alocare eșuată pentru elemente!\n");
            end_ncurses(win1, win);
        }
        elemente = temp;
        elemente[nEl] = strdup(token);
        if (!elemente[nEl]) 
        {
            free(tmp);
            handle_error(win, "Eroare: Alocare eșuată pentru elemente[%d]!\n", nEl);
            end_ncurses(win1, win);
        }
        nEl++;
        token = strtok(NULL, " ");
    }
    free(tmp);

    // for(int i = 0; i < nEl; i++)
    // {
    //     display_message(win, "%s ", elemente[i]);
    // }
    
    uint32_t instruction = UINT32_MAX;
	if (nEl == 4) 
	{
	    if((strcasecmp(elemente[0], "hlt") == 0) || (strcasecmp(elemente[0], "ret") == 0) || (strcasecmp(elemente[0], "mov") == 0) 
	        || (strcasecmp(elemente[0], "lda") == 0) || (strcasecmp(elemente[0], "ldr") == 0) || (strcasecmp(elemente[0], "asr") == 0)
            || (strcasecmp(elemente[0], "str") == 0) || (strcasecmp(elemente[0], "sta") == 0) || (strcasecmp(elemente[0], "psh") == 0) 
            || (strcasecmp(elemente[0], "pop") == 0) || (strcasecmp(elemente[0], "bra") == 0) || (strcasecmp(elemente[0], "brp") == 0)
            || (strcasecmp(elemente[0], "brz") == 0) || (strcasecmp(elemente[0], "bmi") == 0) || (strcasecmp(elemente[0], "bpl") == 0)
            || (strcasecmp(elemente[0], "bvs") == 0) || (strcasecmp(elemente[0], "bcs") == 0) || (strcasecmp(elemente[0], "jms") == 0))
	    {
            handle_error(win, "Eroare: Instructiunea '%s' nu este valida cu 4 parametri -> linia %d\n", elemente[0], lAcum);
            end_ncurses(win1, win);
        }
	    if (elemente[2][0] == '#') 
	    {
	        int nr = strtol(&elemente[2][1], NULL, 10);
	        instruction = encode_I_instruction(elemente[0], elemente[1][1] - '0', nr, elemente[3][1] - '0', 1);
	    }
	    else if(elemente[3][0] == '#') 
	    {
	        int nr = strtol(&elemente[3][1], NULL, 10);
	        instruction = encode_I_instruction(elemente[0], elemente[1][1] - '0', elemente[2][1] - '0', nr, 1);
	    }
        else if((strcasecmp(elemente[0], "beq") == 0) || (strcasecmp(elemente[0], "bne") == 0) || (strcasecmp(elemente[0], "blt") == 0)
                || (strcasecmp(elemente[0], "ble") == 0) || (strcasecmp(elemente[0], "bgt") == 0) || (strcasecmp(elemente[0], "bgeu") == 0)
                || (strcasecmp(elemente[0], "bltu") == 0))
        {
            instruction = encode_BR_instruction(elemente[0], elemente[3], elemente[1][1] - '0', elemente[2][1] - '0', 1);
        }
	    else 
	    {
	        instruction = encode_R_instruction(elemente[0], elemente[1][1] - '0', elemente[2][1] - '0', elemente[3][1] - '0', 1);
	    }
	}

	else if (nEl == 3) 
	{
        if((strcasecmp(elemente[0], "hlt") == 0) || (strcasecmp(elemente[0], "ret") == 0) || (strcasecmp(elemente[0], "psh") == 0) 
            || (strcasecmp(elemente[0], "pop") == 0) || (inArray(elemente[0], branches, sizeof(branches)/sizeof(branches[0])) == 1))
        {
            handle_error(win, "Eroare: Instructiunea '%s' nu este valida cu 3 parametri -> linia %d\n", elemente[0], lAcum);
            end_ncurses(win1, win);
        }
	    else if(strcasecmp(elemente[0], "ldr") == 0 || strcasecmp(elemente[0], "str") == 0)
	    {
            int nr = 0; 
            int reg_src = 0; 
            char *offset_ptr = strchr(elemente[2], '('); // prima paranteza
            char *reg_ptr = strchr(elemente[2], ')'); // a doua paranteza

            if (offset_ptr && reg_ptr && reg_ptr > offset_ptr) 
            {
                // offset-ul
                *offset_ptr = '\0';
                if (strlen(elemente[2]) > 0) 
                {
                    for (int i = 0; i < strlen(elemente[2]); i++)
                    {
                        if (!isdigit(elemente[2][i]))
                        {
                            handle_error(win, "Eroare la linia %s\nOffset invalid pentru instructiunea '%s'!\n", linie, elemente[0]);
                            end_ncurses(win1, win);
                        }
                        nr = nr * 10 + (elemente[2][i] - '0');
                    }
                }

                // verif registru
                if ((offset_ptr[1] == 'R' || offset_ptr[1] == 'r') && isdigit(offset_ptr[2]) && offset_ptr[3] == ')')
                {
                    reg_src = offset_ptr[2] - '0';
                }
                else
                {
                    handle_error(win, "Eroare la linia %s\nFormat invalid pentru instructiunea '%s'!\n", linie, elemente[0]);
                    end_ncurses(win1, win);
                }
            }
            else if (!offset_ptr && !reg_ptr && elemente[2][0] == '(' && (elemente[2][1] == 'r' || elemente[2][1] == 'R')
                    && isdigit(elemente[2][2]) && elemente[2][3] == ')') 
            {
                // Forma ldr r1, (r2) fara offset
                nr = 0;
                reg_src = elemente[2][2] - '0';
            }   
            else
            {
                handle_error(win, "Eroare la linia %s\nFormat necunoscut pentru instructiunea '%s'!\n", linie, elemente[0]);
                end_ncurses(win1, win);
            }
            instruction = encode_I_instruction(elemente[0], elemente[1][1] - '0', reg_src, nr, 1);
	    }
        else if(strcasecmp(elemente[0], "lda") == 0 || strcasecmp(elemente[0], "sta") == 0)
        {
	        int nr = strtol(&elemente[2][1], NULL, 10);
	        instruction = encode_I_instruction(elemente[0], elemente[1][1] - '0', 0, nr, 1);   
        }
        else if(elemente[2][0] == '#') 
	    {
	        int nr = strtol(&elemente[2][1], NULL, 10);
	        instruction = encode_I_instruction(elemente[0], elemente[1][1] - '0', 0, nr, 0);
	    }
	    else
	    {
	        instruction = encode_R_instruction(elemente[0], elemente[1][1] - '0', 0, elemente[2][1] - '0', 0);
	    }
	}

    else if (nEl == 2) 
    {
        if ((strcasecmp(elemente[0], "bra") == 0) || (strcasecmp(elemente[0], "brp") == 0) || (strcasecmp(elemente[0], "brz") == 0) 
            || (strcasecmp(elemente[0], "bmi") == 0) || (strcasecmp(elemente[0], "bpl") == 0) || (strcasecmp(elemente[0], "bvs") == 0) 
            || (strcasecmp(elemente[0], "bcs") == 0) || (strcasecmp(elemente[0], "jms") == 0)) 
        {
            instruction = encode_BR_instruction(elemente[0], elemente[1], 0, 0, 0);
        }
        else if ((strcasecmp(elemente[0], "psh") == 0) || (strcasecmp(elemente[0], "pop") == 0))
        {
            instruction = encode_I_instruction(elemente[0], elemente[1][1] - '0', 0, 0, 0);
        }
        else 
        {
            handle_error(win, "Eroare: Instructiunea '%s' nu este valida cu 2 parametri -> linia %d\n", elemente[0], lAcum);
            end_ncurses(win1, win);
        }
    }

    else if (nEl == 1) 
    {
        if (strcasecmp(elemente[0], "hlt") == 0) 
        {
            instruction = encode_R_instruction(elemente[0], 0, 0, 0, 0);
        }
        else if (strcasecmp(elemente[0], "ret") == 0)
        {
            instruction = encode_I_instruction(elemente[0], 0, 0, 0, 0);
        }
        else 
        {
            handle_error(win, "Eroare: Instrucțiunea '%s' nu este validă cu 1 parametru -> linia %d\n", elemente[0], lAcum);
            end_ncurses(win1, win);
        }
    }

    if (instruction == UINT32_MAX) 
    {
        handle_error(win, "Eroare la codificarea instructiunii -> linia %d\n", lAcum);
        end_ncurses(win1, win);

    }
    instructiuni[lAcum] = instruction;

    if (elemente) 
    {
        for (int i = 0; i < nEl; i++) 
        {
            free(elemente[i]);
        }
        free(elemente);
    }
}

void parse_lines(char **lines, int num_lines, WINDOW *winL, WINDOW *winR) 
{
    win = winR;
    win1 = winL;
    char **prelucrate = malloc(num_lines * sizeof(char *));
    if (!prelucrate) 
    {
        handle_error(win, "Eroare la alocarea memoriei pentru linii prelucrate!\n");
        end_ncurses(win1, win);
    }
    for (int i = 0; i < num_lines; i++) 
    {
        char *linie_curenta = lines[i];
        if (*linie_curenta == '\0' || *linie_curenta == '\n') // linie goala sau doar newline
        {
            continue;
        }

        // display_message(win, "Procesare linie: %s\n", lines[i]);
        char *linie_prelucrata = prelucrandoEtichete(lines[i]);
        if (linie_prelucrata) 
        {
            prelucrate[iCount++] = linie_prelucrata;
            // display_message(win, "Linie prelucrata: %s\n", linie_prelucrata);
        }
    }

    instructiuni = malloc(iCount * sizeof(uint32_t));
    for (int i = 0; i < iCount; i++) 
    {
        if (prelucrate[i]) 
        {
            procesandoInstructiuni(prelucrate[i], i);
        }
    }

    // decodare + executie
    decode_instructions(instructiuni, iCount, winL, winR);

    for(int i = 0; i < iCount; i++)
    {
        free(prelucrate[i]);
    }
    free(prelucrate);
    free(instructiuni);
}
