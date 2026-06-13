#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/*
 * TODO list:
 *  - after every loop remove the last loop output so it only shows
 *  the output from new expressions (this is almost done, just need to remove
 *  last output by character instead of by line, because it remove output
 *  that it shouldn't if the last output didn't have a break-line)
 *
 *  - add ability to use arrow up and arrow down to check history and
 *  left arrow and right arrow to edit the text
 *
 *  - if the users adds an expression that returns a value but they don't catch the value
 *  we should print that return value
 *
 *  - add ability do edit history/current backing file
 */

static int lastOutputLength = 0;

typedef struct {
    char buffer[256];
} OutputLine;

bool compileAndRun(char *tempFilePath) {
    char sysCall[256] = "gcc ";
    strcat(sysCall, tempFilePath);
    strcat(sysCall, " -o repl 2>&1 && ./repl 2>&1 ");
    FILE *pipe = popen(sysCall, "r");

    OutputLine outputLine;
    OutputLine allLines[256];
    int lineCount = 0;
    while (fgets(outputLine.buffer, sizeof(outputLine.buffer), pipe)) {
        allLines[lineCount++] = outputLine;
    }

    int status = pclose(pipe);
    if (WIFEXITED(status)) { // process terminated normally (did not crash)
        int exitCode = WEXITSTATUS(status);
        if (exitCode == 0) {
            for (int i = lastOutputLength; i < lineCount; ++i) {
                printf("%s\n", allLines[i].buffer);
            }
            //TODO: figure out a way to do this without removing output if
            //the last output didn't have a break line on it's last line
            //probably need to do this by character instead of line
            lastOutputLength = lineCount;
        } else {
            // gcc return non zero exit code (could also be the users program)

            // Print all output to show complilation errors
            for (int i = 0; i < lineCount; ++i) {
                printf("%s", allLines[i].buffer);
            }
            printf("gcc exit code non-zero\n");
            return 0;
        }
    } else {
        //TODO: Log
        return 0;
    }

    return 1;
}

int main() {
    char *tempFilePath = "./temp.c";
    FILE *codeFile = fopen(tempFilePath, "w");

    if (codeFile == 0) {
        printf("Error creating code file\n");
        return 1;
    }

    fprintf(codeFile, "#include <stdio.h>\nint main() {\n");
    char lastExpression[256];
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

        if (!compileAndRun(tempFilePath)) {
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
