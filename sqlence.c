/*** includes ***/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/*** data structures ***/

typedef struct {
    char *buffer;
    size_t bufferLength; // unsigned - type to hold positive array indices
    ssize_t inputLength; // signed - type to hold integer array indices
} InputBuffer;

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED
} MetaCommandResult;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum {
    STATEMENT_INSERT, 
    STATEMENT_SELECT
} StatementType;

typedef struct {
    StatementType type;
} Statement;

/*** function declarations ***/

InputBuffer* newInputBuffer();
void printPrompt();
void readInputBuffer(InputBuffer* inputBuffer);
void closeInputBuffer(InputBuffer* inputBuffer);
MetaCommandResult doMetaCommand(InputBuffer* inputBuffer);
PrepareResult prepareResult(InputBuffer* inputBuffer, Statement* statement);

/*** function definitions ***/

InputBuffer* newInputBuffer() {
    InputBuffer* inputBuffer = (InputBuffer *)malloc(sizeof(InputBuffer));
    inputBuffer->buffer = NULL;
    inputBuffer->bufferLength = 0;
    inputBuffer->inputLength = 0;

    return inputBuffer;
}

void printPrompt() {
    printf("db >>> ");
}

void readInputBuffer(InputBuffer* inputBuffer) {
    ssize_t bytes_read = getline(&(inputBuffer->buffer), &(inputBuffer->bufferLength), stdin);

    if (bytes_read <= 0)
    {
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }

    // Ignore trailing newline
    inputBuffer->inputLength = bytes_read - 1;
    inputBuffer->buffer[bytes_read - 1] = 0;
}

void closeInputBuffer(InputBuffer* inputBuffer) {
    free(inputBuffer->buffer);
    free(inputBuffer);
}

MetaCommandResult doMetaCommand(InputBuffer* inputBuffer) {
    if (strcmp(inputBuffer->buffer, ".exit") == 0) {
        exit(EXIT_SUCCESS);
    } else {
        return META_COMMAND_UNRECOGNIZED;
    }
}

PrepareResult prepareStatement(InputBuffer* inputBuffer, Statement* statement) {
    if (strncmp(inputBuffer->buffer, "insert", 6) == 0) {
        statement->type = STATEMENT_INSERT;
        return PREPARE_SUCCESS;
    }
    if (strcmp(inputBuffer->buffer, "select") == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void executeStatement(Statement* statement) {
    switch (statement->type) {
        case (STATEMENT_INSERT):
            printf("Insert.\n");
            break;
        case (STATEMENT_SELECT):
            printf("Select.\n");
            break;
    }
}

/*** init ***/

int main(int argc, char* argv[]) {
    InputBuffer* inputBuffer = newInputBuffer();

    while (true) {
        printPrompt();
        readInputBuffer(inputBuffer);

        if (inputBuffer->buffer[0] == '.') {
            switch (doMetaCommand(inputBuffer)) {
                case (META_COMMAND_SUCCESS):
                    continue;
                case (META_COMMAND_UNRECOGNIZED):
                    printf("Unrecognized command '%s'\n", inputBuffer->buffer);
                    continue;
            }
        }

        Statement statement;
        switch (prepareStatement(inputBuffer, &statement)) {
            case (PREPARE_SUCCESS):
                break;
            case (PREPARE_UNRECOGNIZED_STATEMENT):
                printf("Unrecognized keyword at start of '%s'.\n", inputBuffer->buffer);
                continue;
        }

        executeStatement(&statement);
        printf("Executed.\n");
    }

    printf("\n");
    return EXIT_SUCCESS;
}