/*** includes ***/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/*** data structures ***/

typedef struct
{
    char *buffer;
    size_t bufferLength; // unsigned - type to hold positive array indices
    ssize_t inputLength; // signed - type to hold integer array indices
} InputBuffer;

/*** function definitions ***/

InputBuffer *newInputBuffer();
void printPrompt();
void readInputBuffer(InputBuffer *inputBuffer);
void closeInputBuffer(InputBuffer *inputBuffer);

/*** function declarations ***/

InputBuffer *newInputBuffer()
{
    InputBuffer *inputBuffer = (InputBuffer *)malloc(sizeof(InputBuffer));
    inputBuffer->buffer = NULL;
    inputBuffer->bufferLength = 0;
    inputBuffer->inputLength = 0;

    return inputBuffer;
}

void printPrompt()
{
    printf("db >>> ");
}

void readInputBuffer(InputBuffer *inputBuffer)
{
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

void closeInputBuffer(InputBuffer *inputBuffer)
{
    free(inputBuffer->buffer);
    free(inputBuffer);
}

/*** init ***/

int main(int argc, char *argv[])
{
    InputBuffer *inputBuffer = newInputBuffer();

    while (true)
    {
        printPrompt();
        readInputBuffer(inputBuffer);

        if (strcmp(inputBuffer->buffer, ".exit") == 0)
        {
            closeInputBuffer(inputBuffer);
            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("Unrecognized command '%s'.\n\n", inputBuffer->buffer);
        }
    }

    printf("\n");
    return EXIT_SUCCESS;
}