/* TODO:
 *  - printing seems fucked, fix
 * */


#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

//#define NUM_ROWS 80
//#define NUM_COLUMNS 25
#define NUM_ROWS 25
#define NUM_COLUMNS 80

#define TO_ASCII(x) ((x) + 0x30)
#define FROM_ASCII(x) ((x) - 0x30)
#define IS_NUM(c) ((c) >= 0x30 && (c) <= 0x39)
#define IS_CHAR(c) (((c) >= 0x61 && (c) <= 0x7a) || ((c) >= 0x61 && (c) <= 0x7a))

#define NEXT_INSTRUCTION goto *(void **) (pc->addr)

typedef signed long int data_byte;

typedef enum {
    COMMAND_ADD,
    COMMAND_SUB,
    COMMAND_DIV,
    COMMAND_MUL,
    COMMAND_MOD,
    COMMAND_NOT,
    COMMAND_DUP,
    COMMAND_RIGHT,
    COMMAND_LEFT,
    COMMAND_UP,
    COMMAND_DOWN,
    COMMAND_RAND,
    COMMAND_NULL,
    COMMAND_IFHOR,
    COMMAND_IFVERT,
    COMMAND_READNUM,
    COMMAND_READASC,
    COMMAND_WRITNUM,
    COMMAND_WRITASC,
    COMMAND_BRIDGE,
    COMMAND_POP,
    COMMAND_SWP,
    COMMAND_GT,
    COMMAND_GETPL,
    COMMAND_SETPL,
    COMMAND_STRINGM,
    COMMAND_END,
    COMMAND_CNT
} command_type;

typedef enum {
    DIR_RIGHT,
    DIR_LEFT,
    DIR_UP,
    DIR_DOWN,
    DIR_CNT
} pc_direction;

typedef struct {
    char c;
    void* addr;
} byte;

byte program[NUM_ROWS][NUM_COLUMNS];
bool inStringMode = false;

typedef struct stack_cell_t {
    data_byte val;
    struct stack_cell_t* next;
} stack_cell;

typedef struct {
    stack_cell* top;
} stack;

void Push(stack* currentStack, data_byte toPush) {
    assert(currentStack);

    if (currentStack->top == NULL) {
        currentStack->top = (stack_cell *) malloc (sizeof(stack_cell));
        currentStack->top->val = toPush;
        currentStack->top->next = NULL;
    }else {
        stack_cell *temp = (stack_cell *) malloc (sizeof(stack_cell));
        temp->val = toPush;
        temp->next = currentStack->top;
        currentStack->top = temp;
    }
}

data_byte Pop(stack* currentStack) {
    assert(currentStack);

    if (currentStack->top == NULL) {
        Push(currentStack, 0);
        return 0;
    }

    stack_cell *temp = currentStack->top;
    data_byte res = temp->val;
    currentStack->top = currentStack->top->next;

    free(temp);

    return res;
}

void Examine(stack* currentStack) {
    assert(currentStack);

    printf("DEBUG: start of stack trace\n");
    stack_cell *temp = currentStack->top;
    while (temp != NULL) {
        printf("val: %ld ASCII: %c\n", temp->val, (char) temp->val);
        temp = temp->next;
    }

    printf("DEBUG: end of stack trace\n");
}

data_byte Peek(stack* currentStack) {
    assert(currentStack);

    if (currentStack->top == NULL) {
        Push(currentStack, 0);
        return 0;
    }

    return currentStack->top->val;
}

void ReadProgram(FILE *input) {
    char c;
    int i = 0, j = 0;

    for (int row = 0; row < NUM_ROWS; row++)
        for (int column = 0; column < NUM_COLUMNS; column++) {
            program[row][column].c = ' ';
        }

    while ((c = fgetc(input)) != EOF) {
        if (c == '\n') {
            i++;
            j = 0;
        }else {
            if ((IS_CHAR(c) && (c != 'p' || c != 'q')) || IS_NUM(c)) {
            //if (c >= 0x30 && c <= 0x39 || (c >= )) {
                program[i][j++].c = c;
            }else {
                program[i][j++].c = c;
            }
        }
    }
}

void UpdatePosition(int* currentRow, int* currentColumn, pc_direction currentDirection) { //why is this even out here?
    if (currentDirection == DIR_RIGHT) {
        *currentColumn = (*currentColumn + 1) % NUM_COLUMNS;
    }else if (currentDirection == DIR_LEFT) {
        *currentColumn = (*currentColumn - 1 >= 0) ? *currentColumn - 1 : NUM_COLUMNS - 1;
    }else if (currentDirection == DIR_DOWN) {
        *currentRow = (*currentRow + 1) % NUM_ROWS;
    }else if (currentDirection == DIR_UP) {
        *currentRow = (*currentRow - 1 >= 0) ? *currentRow - 1 : NUM_ROWS - 1;
    }
}

void UpdateAddresses(void *labelTab[]) {
    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLUMNS; j++) {
            switch (program[i][j].c) {
                case '+':
                    program[i][j].addr = labelTab[(int)COMMAND_ADD];
                    break;
                case '-':
                    program[i][j].addr = labelTab[(int)COMMAND_SUB];
                    break;
                case '*':
                    program[i][j].addr = labelTab[(int)COMMAND_MUL];
                    break;
                case '/':
                    program[i][j].addr = labelTab[(int)COMMAND_DIV];
                    break;
                case '%':
                    program[i][j].addr = labelTab[(int)COMMAND_MOD];
                    break;
                case '!':
                    program[i][j].addr = labelTab[(int)COMMAND_NOT];
                    break;
                case ':':
                    program[i][j].addr = labelTab[(int)COMMAND_DUP];
                    break;
                case '>':
                    program[i][j].addr = labelTab[(int)COMMAND_RIGHT];
                    break;
                case '<':
                    program[i][j].addr = labelTab[(int)COMMAND_LEFT];
                    break;
                case '^':
                    program[i][j].addr = labelTab[(int)COMMAND_UP];
                    break;
                case 'v':
                    program[i][j].addr = labelTab[(int)COMMAND_DOWN];
                    break;
                case '?':
                    program[i][j].addr = labelTab[(int)COMMAND_RAND];
                    break;
                case ' ':
                    program[i][j].addr = labelTab[(int)COMMAND_NULL];
                    break;
                case '_':
                    program[i][j].addr = labelTab[(int)COMMAND_IFHOR];
                    break;
                case '|':
                    program[i][j].addr = labelTab[(int)COMMAND_IFVERT];
                    break;
                case '&':
                    program[i][j].addr = labelTab[(int)COMMAND_READNUM];
                    break;
                case '~':
                    program[i][j].addr = labelTab[(int)COMMAND_READASC];
                    break;
                case '.':
                    program[i][j].addr = labelTab[(int)COMMAND_WRITNUM];
                    break;
                case ',':
                    program[i][j].addr = labelTab[(int)COMMAND_WRITASC];
                    break;
                case '#':
                    program[i][j].addr = labelTab[(int)COMMAND_BRIDGE];
                    break;
                case '$':
                    program[i][j].addr = labelTab[(int)COMMAND_POP];
                    break;
                case '\\':
                    program[i][j].addr = labelTab[(int)COMMAND_SWP];
                    break;
                case '`':
                    program[i][j].addr = labelTab[(int)COMMAND_GT];
                    break;
                case 'g':
                    program[i][j].addr = labelTab[(int)COMMAND_GETPL];
                    break;
                case 'p':
                    program[i][j].addr = labelTab[(int)COMMAND_SETPL];
                    break;
                case '"':
                    program[i][j].addr = labelTab[(int)COMMAND_STRINGM];
                    break;
                case '@':
                    program[i][j].addr = labelTab[(int)COMMAND_END];
                    break;
                default:
                    program[i][j].addr = labelTab[(int)COMMAND_CNT];
                    break;
            }
        }
    }
}

void Run() {
    static void *labelTab[] = {
        &&addLabel,
        &&subLabel,
        &&divLabel,
        &&mulLabel,
        &&modLabel,
        &&notLabel,
        &&dupLabel,
        &&rightLabel,
        &&leftLabel,
        &&upLabel,
        &&downLabel,
        &&randLabel,
        &&nullLabel,
        &&ifhorLabel,
        &&ifvertLabel,
        &&readNumLabel,
        &&readAscLabel,
        &&writeNumLabel,
        &&writeAscLabel,
        &&bridgeLabel,
        &&popLabel,
        &&swpLabel,
        &&gtLabel,
        &&getPLLabel,
        &&setPLLabel,
        &&stringmLabel,
        &&endLabel,
        &&defaultLabel,
    };

    byte* pc;
    int currentRow = 0, currentColumn = 0;
    pc_direction currentDirection = DIR_RIGHT;
    stack* programStack = (stack*) malloc(sizeof(stack));
    programStack->top = NULL;

    srand(time(NULL));
    UpdateAddresses(labelTab);

    for (;;) {
        pc = &program[currentRow][currentColumn];

        if (!inStringMode) {
            switch (pc->c) {
                case '+':
addLabel:
                    {
                        data_byte op2 = Pop(programStack);
                        data_byte op1 = Pop(programStack);
                        data_byte res = op1 + op2;

                        Push(programStack, res);

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '-':
subLabel:
                    {
                        data_byte op2 = Pop(programStack);
                        data_byte op1 = Pop(programStack);
                        data_byte res = op1 - op2;

                        Push(programStack, res);

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '*':
mulLabel:
                    {
                        data_byte op2 = Pop(programStack);
                        data_byte op1 = Pop(programStack);
                        data_byte res = op1 * op2;

                        Push(programStack, res);

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '/':
divLabel:
                    {
                        data_byte op2 = Pop(programStack);
                        data_byte op1 = Pop(programStack);
                        data_byte res = op1 / op2;

                        Push(programStack, res);

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '%':
modLabel:
                    {
                        data_byte op2 = Pop(programStack);
                        data_byte op1 = Pop(programStack);
                        data_byte res = op1 % op2;

                        Push(programStack, res);

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '!':
notLabel:
                    {
                        data_byte op = Pop(programStack);

                        if (op == 0)
                            Push(programStack, 1);
                        else
                            Push(programStack, 0);

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case ':':
dupLabel:
                    {
                        data_byte op = Peek(programStack);

                        Push(programStack, op);

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '>':
rightLabel:
                    {
                        currentDirection = DIR_RIGHT;

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '<':
leftLabel:
                    {
                        currentDirection = DIR_LEFT;

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '^':
upLabel:
                    {
                        currentDirection = DIR_UP;

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case 'v':
downLabel:
                    {
                        currentDirection = DIR_DOWN;

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '?':
randLabel:
                    {
                        currentDirection = rand() % 4;

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case ' ':
nullLabel:
                    {
                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '_':
ifhorLabel:
                    {
                        //Examine(programStack);
                        data_byte cond = Pop(programStack);

                        if (cond)
                            currentDirection = DIR_LEFT;
                        else
                            currentDirection = DIR_RIGHT;

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '|':
ifvertLabel:
                    {
                        data_byte cond = Pop(programStack);

                        if (cond)
                            currentDirection = DIR_UP;
                        else
                            currentDirection = DIR_DOWN;

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '&':
readNumLabel:
                    {
                        data_byte inp;

                        scanf(" %ld", &inp);
                        Push(programStack, inp);

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '~':
readAscLabel:
                    {
                        char c;

                        scanf(" %c", &c);
                        Push(programStack, c);

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '.':
writeNumLabel:
                    {
                        data_byte num = Pop(programStack);

                        printf("%ld", num);

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case ',':
writeAscLabel:
                    {
                        data_byte c = Pop(programStack);

                        printf("%c", (char) c);

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '#':
bridgeLabel:
                    {
                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '$':
popLabel:
                    {
                        Pop(programStack);

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '\\':
swpLabel:
                    {
                        data_byte upper = Pop(programStack);
                        data_byte lower = Pop(programStack);

                        Push(programStack, upper);
                        Push(programStack, lower);

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '`':
gtLabel:
                    {
                        data_byte op2 = Pop(programStack);
                        data_byte op1 = Pop(programStack);

                        op1 > op2 ? Push(programStack, 1) : Push(programStack, 0);

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case 'g':
getPLLabel:
                    {
                        data_byte y = Pop(programStack);
                        data_byte x = Pop(programStack);

                        byte temp = program[y][x];
                        Push(programStack, temp.c);

                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case 'p':
setPLLabel:
                    {
                        data_byte y = Pop(programStack);
                        data_byte x = Pop(programStack);
                        data_byte val = Pop(programStack);

                        program[y][x].c = (char) val;
                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
                case '"':
stringmLabel:
                    {
                        inStringMode = !inStringMode;

                        //update
                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        goto stringModeLabel;
                        break;
                    }
                case '@':
endLabel:
                    goto terminate;
                default:
defaultLabel:
                    {
                        Push(programStack, FROM_ASCII(pc->c));
                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        pc = &program[currentRow][currentColumn];
                        NEXT_INSTRUCTION;
                        break;
                    }
            }
        }else {
stringModeLabel:
            if (pc->c == '"') {
                inStringMode = !inStringMode;
                UpdatePosition(&currentRow, &currentColumn, currentDirection);
                pc = &program[currentRow][currentColumn];

                NEXT_INSTRUCTION;
            } else {
                Push(programStack, pc->c);
                UpdatePosition(&currentRow, &currentColumn, currentDirection);
                pc = &program[currentRow][currentColumn];
                goto stringModeLabel;
            }
        }

    }

terminate:
    return;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("ERROR: befunge source file needed. exiting\n");
        exit(1);
    }

    FILE* inpFile = fopen(argv[1], "r");

    if (!inpFile) {
        perror("main");
        exit(1);
    }

    ReadProgram(inpFile);
    fclose(inpFile);

    Run();

    return 0;
}
