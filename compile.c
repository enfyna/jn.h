#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 128

#define HEADER "jn.part.h"
#define SOURCE "jn.part.c"
#define DEST "jn.h"

int compile(void)
{
    char buf[BUF_SIZE] = { 0 };

    FILE* destination = fopen(DEST, "w+");

    fprintf(destination, "#ifndef JN_H_\n");
    fprintf(destination, "#define JN_H_\n");

    FILE* header = fopen(HEADER, "r");

    memset(buf, 0, sizeof(char) * BUF_SIZE);
    size_t r = 0;
    while ((r = fread(buf, sizeof(char), BUF_SIZE, header)) > 0) {
        fwrite(buf, sizeof(char), r, destination);
    }

    fclose(header);

    fprintf(destination, "\n\n#ifdef JN_IMPLEMENTATION\n\n");

    FILE* source = fopen(SOURCE, "r");

    r = 0;
    while (fgets(buf, BUF_SIZE, source) != NULL) {
        if (strcmp(buf, "#include \"" HEADER "\"\n") == 0) {
            printf("Deleting: %s", buf);
        } else {
            fputs(buf, destination);
        }
    }

    fclose(source);

    fprintf(destination, "\n#endif // JN_IMPLEMENTATION\n");
    fprintf(destination, "#endif // JN_H_\n");

    fclose(destination);
    return 0;
}

int extract(void)
{
    char buf[BUF_SIZE] = { 0 };

    FILE* destination = fopen(DEST, "r");

    FILE* header = fopen(HEADER, "w+");
    FILE* source = fopen(SOURCE, "w+");

    enum Section { STR,
        HDR,
        SRC,
        END,
    } m
        = STR;

    while (fgets(buf, BUF_SIZE, destination) != NULL) {
        if (strcmp(buf, "#define JN_H_\n") == 0) {
            m = HDR;
            continue;
        } else if (strcmp(buf, "#ifdef JN_IMPLEMENTATION\n") == 0) {
            m = SRC;
            const char* include = "#include \"" HEADER "\"\n";
            fputs(include, source);
            continue;
        } else if (strcmp(buf, "#endif // JN_IMPLEMENTATION\n") == 0) {
            break;
        }

        if (m == HDR) {
            fputs(buf, header);
        } else if (m == SRC) {
            fputs(buf, source);
        }
    }

    fclose(header);
    fclose(source);
    fclose(destination);
    return 0;
}

int main(int argc, char* argv[])
{
    (void)argv;

    int ignore = 0;

    if (argc == 2) {
        printf("Compile ?\n");
        scanf("%d", &ignore);
        return compile();
    } else {
        printf("Extract ?\n");
        scanf("%d", &ignore);
        return extract();
    }
}
