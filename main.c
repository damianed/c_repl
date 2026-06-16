#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/*
 * TODO list:
 *  - add ability to use arrow up and arrow down to check history and
 *  left arrow and right arrow to edit the text
 *
 *  - if the users adds an expression that returns a value but they don't catch the value
 *  we should print that return value
 *
 *  - add ability do edit history/current backing file (temp.c)
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

    int charCount = 0;
    int character;
    unsigned char characters[256*256];
    while ((character = fgetc(pipe)) != EOF) {
        characters[charCount++] = (unsigned char) character;
    }

    int status = pclose(pipe);
    if (WIFEXITED(status)) { // process terminated normally (did not crash)
        int exitCode = WEXITSTATUS(status);
        if (exitCode == 0) {
            for (int i = lastOutputLength; i < charCount; ++i) {
                printf("%c", characters[i]);
            }
            lastOutputLength = charCount;
        } else {
            // gcc return non zero exit code (could also be the users program)

            // Print all output to show complilation errors
            printf("%s", characters);
            printf("gcc exited with non-zero exit code: %d\n", exitCode);
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
        lastExpression[strcspn(lastExpression, "\n")] = '\0';
        if (strcmp(lastExpression, "exit();") == 0) {
            break;
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

    fclose(codeFile);
    remove(tempFilePath);
    return 0;
}
