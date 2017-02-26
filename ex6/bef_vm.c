/*NOTE: there needs to be a var telling us the direction we are currently moving in wrt
 * the program counter
 * Also, we can either read a program in befunge, and convert it to some intermediate representation,
 * which we then pass to Run(), or keep it as is, and pass that to Run()... pros/cons of each approach?
 * */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define NUM_ROWS 80
#define NUM_COLUMNS 25

#define TO_ASCII(x) (x) + 0x30
#define FROM_ASCII(x) (x) - 0x30

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
    bool isCommand;
    char c;
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
            program[row][column].isCommand = false;
            program[row][column].c = ' ';
        }

    while ((c = fgetc(input)) != EOF) {
        if (c == '\n') {
            i++;
            j = 0;
        }else {
            if (c >= 0x30 && c <= 0x39) {
                program[i][j].isCommand = false;
                program[i][j++].c = c;
            }else {
                program[i][j].isCommand = true;
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

void Run() {
    byte* pc;
    int currentRow = 0, currentColumn = 0;
    pc_direction currentDirection = DIR_RIGHT;
    stack* programStack = (stack*) malloc(sizeof(stack));

    srand(time(NULL));

    for (;;) {
        pc = &program[currentRow][currentColumn];

        if (!pc->isCommand) {
            if (inStringMode)
                Push(programStack, pc->c);
            else
                Push(programStack, FROM_ASCII(pc->c));
        }else {
            switch (pc->c) {
                case '+':
                    {
                        data_byte op2 = Pop(programStack);
                        data_byte op1 = Pop(programStack);
                        data_byte res = op1 + op2;

                        Push(programStack, res);
                        break;
                    }
                case '-':
                    {
                        data_byte op2 = Pop(programStack);
                        data_byte op1 = Pop(programStack);
                        data_byte res = op1 - op2;

                        Push(programStack, res);
                        break;
                    }
                case '*':
                    {
                        data_byte op2 = Pop(programStack);
                        data_byte op1 = Pop(programStack);
                        data_byte res = op1 * op2;

                        Push(programStack, res);
                        break;
                    }
                case '/':
                    {
                        data_byte op2 = Pop(programStack);
                        data_byte op1 = Pop(programStack);
                        data_byte res = op1 / op2;

                        Push(programStack, res);
                        break;
                    }
                case '%':
                    {
                        data_byte op2 = Pop(programStack);
                        data_byte op1 = Pop(programStack);
                        data_byte res = op1 % op2;

                        Push(programStack, res);
                        break;
                    }
                case '!':
                    {
                        data_byte op = Pop(programStack);

                        if (op == 0)
                            Push(programStack, 1);
                        else
                            Push(programStack, 0);

                        break;
                    }
                case ':':
                    {
                        data_byte op = Peek(programStack);

                        Push(programStack, op);

                        break;
                    }
                case '>':
                    {
                        currentDirection = DIR_RIGHT;

                        break;
                    }
                case '<':
                    {
                        currentDirection = DIR_LEFT;

                        break;
                    }
                case '^':
                    {
                        currentDirection = DIR_UP;

                        break;
                    }
                case 'v':
                    {
                        currentDirection = DIR_DOWN;

                        break;
                    }
                case '?':
                    {
                        currentDirection = rand() % 4;

                        break;
                    }
                case ' ':
                    {
                        break;
                    }
                case '_':
                    {
                        data_byte cond = Pop(programStack);

                        if (cond)
                            currentDirection = DIR_LEFT;
                        else
                            currentDirection = DIR_RIGHT;

                        break;
                    }
                case '|':
                    {
                        data_byte cond = Pop(programStack);

                        if (cond)
                            currentDirection = DIR_UP;
                        else
                            currentDirection = DIR_DOWN;

                        break;
                    }
                case '&':
                    {
                        data_byte inp;

                        scanf(" %ld", &inp);
                        Push(programStack, inp);

                        break;
                    }
                case '~':
                    {
                        char c;

                        scanf(" %c", &c);
                        Push(programStack, c);

                        break;
                    }
                case '.':
                    {
                        data_byte num = Pop(programStack);

                        printf("%ld\n", num);

                        break;
                    }
                case ',':
                    {
                        data_byte c = Pop(programStack);

                        printf("%c\n", (char) c);

                        break;
                    }
                case '#':
                    {
                        UpdatePosition(&currentRow, &currentColumn, currentDirection);
                        break;
                    }
                case '$':
                    {
                        Pop(programStack);

                        break;
                    }
                case '\\':
                    {
                        data_byte upper = Pop(programStack);
                        data_byte lower = Pop(programStack);

                        Push(programStack, upper);
                        Push(programStack, lower);

                        break;
                    }
                case '`':
                    {
                        data_byte op2 = Pop(programStack);
                        data_byte op1 = Pop(programStack);

                        op1 > op2 ? Push(programStack, 1) : Push(programStack, 0);

                        break;
                    }
                case 'g':
                    {
                        data_byte y = Pop(programStack);
                        data_byte x = Pop(programStack);

                        byte temp = program[y][x];
                        Push(programStack, temp.c);

                        break;
                    }
                case 'p':
                    {
                        data_byte y = Pop(programStack);
                        data_byte x = Pop(programStack);
                        data_byte val = Pop(programStack);

                        if (!(val >= 0x30 && val <= 0x39))
                            program[y][x].isCommand = true;
                        else
                            program[y][x].isCommand = false;

                        program[y][x].c = (char) val;
                        break;
                    }
                case '"':
                    {
                        inStringMode = !inStringMode;

                        break;
                    }
                case '@':
                    goto terminate;
                default:
                    break;
            }

        }

        UpdatePosition(&currentRow, &currentColumn, currentDirection);
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
