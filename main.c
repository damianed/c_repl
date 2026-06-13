#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * TODO list:
 *  - after every loop remove the last loop output so it only shows
 *  the output from new expressions
 *
 *  - if the compliation fails remove all input added during that loop from the file
 *
 *  - add ability to use arrow up and arrow down to check history
 *
 *  - if the users adds an expresion that returns a value but they don't catch the value
 *  we should print that return value
 */

int main() {
    char *tempFilePath = "./temp.c";
    FILE *codeFile = fopen(tempFilePath, "w");

    if (codeFile == 0) {
        printf("Error creating code file\n");
        return 1;
    }

    fprintf(codeFile, "#include <stdio.h>\nint main() {\n");
    char lastExpression[256];
    int lastOuputLength = 0;
    printf("C REPL: Enter an expression...\n");
    for (;;) {
        printf(": ");
        fgets(lastExpression, sizeof(lastExpression), stdin);
        //lastExpression[strcspn(lastExpression, "\n")] = '\0';
        if (strcmp(lastExpression, "exit();") == 0) {
            return 0;
        }

        long ptrBeforeNewLine = ftell(codeFile);
        fprintf(codeFile, "%s", lastExpression);
        fprintf(codeFile, "%s", "}\n");
        fflush(codeFile);

        char sysCall[256] = "gcc ";
        strcat(sysCall, tempFilePath);
        strcat(sysCall, " -o repl && ./repl");
        int gccReturnCode = system(sysCall);
        if (gccReturnCode != 0) {
            ftruncate(fileno(codeFile), ptrBeforeNewLine);
            fseek(codeFile, ptrBeforeNewLine, SEEK_SET);
            continue;
        }
        printf("\n");

        // Remove "}\n for next loop"
        long filePointerPos = ftell(codeFile);
        filePointerPos -= 2; // "}\n" is 2 bytes
        ftruncate(fileno(codeFile), filePointerPos);
        fseek(codeFile, filePointerPos, SEEK_SET);
    }
    printf("last expression %s\n", lastExpression);
    return 0;
}
