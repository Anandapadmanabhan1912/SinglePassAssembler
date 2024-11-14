#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct myNode
{
    long offset;
    struct myNode *next;
} myNode;

typedef struct symtabEntries
{
    char name[10];
    int value;
    myNode *ll;
} symtabEntries;

symtabEntries symtab[10]; // hopefully 10 new symbols at max
int top;                  // no of entries in symtab
char str[4];

void writeSymtab();
int searchSymtab(char *symbol);
int searchOpcode(char *opcode);
void addNewReference(int index, long offset);
void resolveReferences(int index, FILE *fptr);

void initialise()
{
    for (int i = 0; i < 10; i++)
    {
        symtab[i].value = -1;
        symtab[i].ll = NULL;
    }
}

int main()
{
    int straddr, opcode;
    int rec_count = 0;
    char symbol[10], instruc[10], operand[10];
    top = 0;
    int index, offset;
    long locctr = 0;
    FILE *fptr1 = fopen("input.txt", "r");
    FILE *fptr2 = fopen("output.txt", "w+");
    fscanf(fptr1, "%s %s %s", symbol, instruc, operand);
    if (strcmp(instruc, "START") == 0)
    {
        fprintf(fptr2, "H %s %s\n", symbol, operand);
        locctr = (int)strtol(operand, NULL, 16);
        straddr = locctr; // start address stored
        printf("Start address: %X", straddr);
        fprintf(fptr2, "T ");

        while (fscanf(fptr1, "%s\t%s\t%s", symbol, instruc, operand) != EOF)
        {
            if (strcmp("-", symbol) == 0)
            {
                opcode = searchOpcode(instruc);
                // write the opcode and address in output.c
                if (opcode != -1)
                {
                    if (rec_count < 6)
                    {
                        if (searchSymtab(operand) != -1)
                        {
                            index = searchSymtab(operand);
                            if (symtab[index].value != -1)
                            {
                                fprintf(fptr2, " %s%X", str, symtab[index].value);
                                rec_count++;
                            }
                            else
                            {
                                fprintf(fptr2, " %s****", str);
                                offset = ftell(fptr2) - 5;
                                printf("\n%d for %s\n", offset, symtab[index].name);
                                addNewReference(index, offset);
                                rec_count++;
                            }
                        }
                        else if (strcmp("-", operand) != 0)
                        {
                            // add new symbol to SYMTAB
                            strcpy(symtab[top].name, operand);                // Correct assignment
                            printf("\nAdded symbol: %s\n", symtab[top].name); // Debug print
                            fprintf(fptr2, "%s**** ", str);
                            offset = ftell(fptr2) - 5;
                            addNewReference(top, offset);
                            top++;
                            rec_count++;
                        }
                        else
                        {
                            printf("");
                        }
                    }
                    else
                    {
                        fprintf(fptr2, "\nT");
                        rec_count = rec_count % 6;
                        // start new text record
                    }
                }
                else
                {
                    printf("\nError in Instruction... NOT in OPTAB ...%s %d\n", instruc, opcode);
                    return 0;
                }
                // assuming all these are instruction of size 3 bytes
                locctr += 3;
            }
            else
            {
                // symbol field is SIGNIFICANT
                if (strcmp("BYTE", instruc) == 0)
                {
                    index = searchSymtab(symbol);
                    symtab[index].value = locctr;
                    resolveReferences(index, fptr2);
                    locctr += 1;
                }
                else if (strcmp("WORD", instruc) == 0)
                {
                    index = searchSymtab(symbol);
                    symtab[index].value = locctr;
                    resolveReferences(index, fptr2);
                    locctr += 3;
                }
                else if (strcmp("RESB", instruc) == 0)
                {
                    index = searchSymtab(symbol);
                    symtab[index].value = locctr;
                    resolveReferences(index, fptr2);
                    locctr += atoi(operand);
                }
                else if (strcmp("RESW", instruc) == 0)
                {
                    index = searchSymtab(symbol);
                    symtab[index].value = locctr;
                    resolveReferences(index, fptr2);
                    locctr += 3 * atoi(operand);
                }
            }
        }
    }
    else
    {
        printf("\nSTART missing .... Terminated...");
    }
    fprintf(fptr2, "\nE 00%X", straddr);
    writeSymtab();
    fclose(fptr1);
    fclose(fptr2);
    return 0;
}

void resolveReferences(int index, FILE *fptr)
{
    myNode *ptr = symtab[index].ll;
    while (ptr != NULL)
    {
        fseek(fptr, ptr->offset, SEEK_SET);
        fprintf(fptr, "%X", symtab[index].value);
        ptr = ptr->next;
    }
    symtab[index].ll = NULL;
    printf("\nAddresses resolved for %s", symtab[index].name);
}

void addNewReference(int index, long offset)
{
    printf("\nAdding reference for symbol: %s at offset: %ld\n", symtab[index].name, offset); // Debug print
    if (symtab[index].ll == NULL)
    {
        symtab[index].ll = (myNode *)malloc(sizeof(myNode));
        symtab[index].ll->offset = offset;
        symtab[index].ll->next = NULL;
    }
    else
    {
        myNode *ptr = symtab[index].ll;
        while (ptr->next != NULL)
        {
            ptr = ptr->next;
        }
        myNode *new = (myNode *)malloc(sizeof(myNode));
        new->offset = offset;
        new->next = NULL;
        ptr->next = new;
    }
}

int searchOpcode(char *opcode)
{
    char mnemonic[5];
    int code;
    FILE *fptr = fopen("optab.txt", "r");
    while (fscanf(fptr, "%s %s", mnemonic, str) != EOF)
    {
        // Convert code to a string and compare with opcode
        if (strcmp(opcode, mnemonic) == 0)
        {
            code = atoi(str);
            return code;
        }
    }
    fclose(fptr);
    return -1;
}

int searchSymtab(char *symbol)
{
    printf("\n");
    for (int i = 0; i < top; i++)
    {
        printf("Checking symbol: %s against %s\n", symbol, symtab[i].name); // Debug print
        if (strcmp(symtab[i].name, symbol) == 0)
        {
            return i;
        }
    }
    return -1;
}

void writeSymtab()
{
    FILE *fptr = fopen("symtab.txt", "w");
    for (int i = 0; i < top; i++)
    {
        fprintf(fptr, "\n%s %X", symtab[i].name, symtab[i].value);
    }
    fclose(fptr);
}