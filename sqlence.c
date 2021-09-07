/*** includes ***/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

/*** data structures ***/

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100
#define sizeOfAttribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

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
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_SYNTAX_ERROR
} PrepareResult;

typedef enum {
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL
} ExecuteResult;

typedef enum {
    STATEMENT_INSERT, 
    STATEMENT_SELECT
} StatementType;

typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

typedef struct {
    StatementType type;
    Row rowToInsert;
} Statement;

const uint32_t ID_SIZE = sizeOfAttribute(Row, id);
const uint32_t USERNAME_SIZE = sizeOfAttribute(Row, username);
const uint32_t EMAIL_SIZE = sizeOfAttribute(Row, email);

const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE/ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct {
    uint32_t numRows;
    void* pages[TABLE_MAX_PAGES];
} Table;

/*** function declarations ***/

void printPrompt();
InputBuffer* newInputBuffer();
void readInputBuffer(InputBuffer* inputBuffer);
void closeInputBuffer(InputBuffer* inputBuffer);
MetaCommandResult doMetaCommand(InputBuffer* inputBuffer);
PrepareResult prepareStatement(InputBuffer* inputBuffer, Statement* statement);
ExecuteResult executeInsert(Statement* statement, Table* table);
ExecuteResult executeSelect(Statement* statement, Table* table);
ExecuteResult executeStatement(Statement* statement, Table* table);
void printRow(Row* row);
void serializeRow(Row* source, void* destination);
void deserializeRow(void* source, Row* destination);
void* rowSlot(Table* table, uint32_t rowNum);
Table* newTable();
void freeTable(Table* table);


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
        int args_assigned = sscanf(inputBuffer->buffer, "insert %d %s %s", &(statement->rowToInsert.id), (statement->rowToInsert.username), (statement->rowToInsert.email));
        if (args_assigned != 3) {
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    }
    if (strcmp(inputBuffer->buffer, "select") == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult executeInsert(Statement* statement, Table* table) {
    if (table->numRows >= TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }

    Row* sourceRow = &(statement->rowToInsert);
    serializeRow(sourceRow, rowSlot(table, table->numRows));
    table->numRows += 1;

    return EXECUTE_SUCCESS;
}

ExecuteResult executeSelect(Statement* statement, Table* table) {
    Row row;
    for (uint32_t i = 0; i < table->numRows; i++) {
        deserializeRow(rowSlot(table, i), &row);
        printRow(&row);
    }
    return EXECUTE_SUCCESS;
}

ExecuteResult executeStatement(Statement* statement, Table* table) {
    switch (statement->type) {
        case (STATEMENT_INSERT):
            return executeInsert(statement, table);
        case (STATEMENT_SELECT):
            return executeSelect(statement, table);
            break;
    }
}

void printRow(Row* row) {
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

void serializeRow(Row* source, void* destination) {
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserializeRow(void* source, Row* destination) {
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

void* rowSlot(Table* table, uint32_t rowNum) {
    uint32_t pageNum = rowNum/ROWS_PER_PAGE;
    void* page = table->pages[pageNum];
    if (page == NULL) {
        // Allocate memory
        page = table->pages[pageNum] = malloc(PAGE_SIZE);
    }
    uint32_t rowOffset = rowNum % ROWS_PER_PAGE;
    uint32_t byteOffset = rowOffset * ROW_SIZE;

    return (page + byteOffset);
}

Table* newTable() {
    Table* table = malloc(sizeof(Table));
    table->numRows = 0;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        table->pages[i] = NULL;
    }
    return table;
}

void freeTable(Table* table) {
    for (int i = 0; table->pages[i]; i++) {
        free(table->pages[i]);
    }
    free(table);
}

/*** init ***/

int main(int argc, char* argv[]) {
    Table* table = newTable();
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
            case (PREPARE_SYNTAX_ERROR):
                printf("Syntax error. Could not parse statement.\n");
                continue;
            case (PREPARE_UNRECOGNIZED_STATEMENT):
                printf("Unrecognized keyword at start of '%s'.\n", inputBuffer->buffer);
                continue;
        }

        switch (executeStatement(&statement, table)) {
            case (EXECUTE_SUCCESS):
                printf("Executed.\n");
                break;
            case EXECUTE_TABLE_FULL:
                printf("Error: Table full.\n");
                break;
        }
    }

    printf("\n");
    return EXIT_SUCCESS;
}