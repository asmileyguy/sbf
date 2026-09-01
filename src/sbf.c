#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define SBF_LINE_MAX    4096
#define SBF_MEM_SIZE    32

#define SBF_INSTRUCTION_NON 0x01 // \0 or space
#define SBF_INSTRUCTION_RIG 0x02 // >
#define SBF_INSTRUCTION_LEF 0x03 // <
#define SBF_INSTRUCTION_INC 0x04 // +
#define SBF_INSTRUCTION_DEC 0x05 // -
#define SBF_INSTRUCTION_PRI 0x06 // .
#define SBF_INSTRUCTION_REA 0x07 // ,
#define SBF_INSTRUCTION_LO1 0x08 // [
#define SBF_INSTRUCTION_LO2 0x09 // ]

typedef struct {
    char c;
    uint8_t instruction;
} instructions_t;

static bool sbf_debug_enabled = false;

static char sbf_lines[SBF_LINE_MAX];
static off_t sbf_ip = 0;

static instructions_t instructions[10] = {
    {'\0', SBF_INSTRUCTION_NON},
    {' ',  SBF_INSTRUCTION_NON},
    {'>',  SBF_INSTRUCTION_RIG},
    {'<',  SBF_INSTRUCTION_LEF},
    {'+',  SBF_INSTRUCTION_INC},
    {'-',  SBF_INSTRUCTION_DEC},
    {'.',  SBF_INSTRUCTION_PRI},
    {',',  SBF_INSTRUCTION_REA},
    {'[',  SBF_INSTRUCTION_LO1},
    {']',  SBF_INSTRUCTION_LO2}
};

static uint8_t mem[SBF_MEM_SIZE]; // Our brainf*ck intreperters memory
static uint32_t memp = 0;

static void sbf_debug(const char *fmt, ...) {
    if (!sbf_debug_enabled) return;
    va_list args;
    va_start(args, fmt);
    printf("sbf: ");
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

static void sbf_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "sbf: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}

static void sbf_enable_debug(void) {
    sbf_debug_enabled = true;
}

static void sbf_usage(const char *prog, bool is_error) {
    if (is_error) {
        fprintf(stderr, "usage for sbf:\n");
        fprintf(stderr, "  %s [options] <file>\n", prog);
        fprintf(stderr, "  options:\n");
        fprintf(stderr, "    -h, --help:  shows this help dialouge\n");
        fprintf(stderr, "    -d, --debug: enables debug messages\n");
        exit(1);
    } else {
        printf("usage for sbf:\n");
        printf("  %s [options] <file>\n", prog);
        printf("  options:\n");
        printf("    -h, --help:  shows this help dialouge\n");
        printf("    -d, --debug: enables debug messages\n");
        exit(0);
    }
}

static void sbf_exec_instruction(uint8_t instruction) {
    // Simple ahh instruction executer
    if (instruction == SBF_INSTRUCTION_NON) {
        // No instruction
    } else if (instruction == SBF_INSTRUCTION_RIG) {
        if (memp == SBF_MEM_SIZE - 1) sbf_error("tried to go out of bounds!");
        memp++;
    } else if (instruction == SBF_INSTRUCTION_LEF) {
        if (memp == 0) sbf_error("tried to go out of bounds!");
        memp--;
    } else if (instruction == SBF_INSTRUCTION_INC) {
        mem[memp]++;
    } else if (instruction == SBF_INSTRUCTION_DEC) {
        mem[memp]--;
    } else if (instruction == SBF_INSTRUCTION_PRI) {
        write(1, &mem[memp], 1);
        fflush(stdout);
    } else if (instruction == SBF_INSTRUCTION_REA) {
        mem[memp] = getchar();
    } else if (instruction == SBF_INSTRUCTION_LO1) {
        if (mem[memp] == 0) {
            int depth = 1;
            while (depth > 0) {
                sbf_ip++;
                if (sbf_ip >= SBF_LINE_MAX || sbf_lines[sbf_ip] == '\0') sbf_error("unmatched [");
                else if (sbf_lines[sbf_ip] == '[') depth++;
                else if (sbf_lines[sbf_ip] == ']') depth--;
            }
        }
    } else if (instruction == SBF_INSTRUCTION_LO2) {
        if (mem[memp] != 0) {
            int depth = 1;
            while (depth > 0) {
                sbf_ip--;
                if (sbf_ip == 0) sbf_error("unmatched ]");
                else if (sbf_lines[sbf_ip] == ']') depth++;
                else if (sbf_lines[sbf_ip] == '[') depth--;
            }
        }
    } else {
        sbf_error("unknown instruction!");
    }
}

static void sbf_next_instruction(char c) {
    for (size_t i = 0; i < sizeof(instructions) / sizeof(instructions[0]); i++) {
        if (c == instructions[i].c) {
            sbf_debug("found instruction '%c', running", instructions[i].c);
            sbf_lines[sbf_ip] = instructions[i].c;
            sbf_exec_instruction(instructions[i].instruction);
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) sbf_error("no filename provided, see --help for help");
    const char *file;
    bool file_found = false;
    #define SBF_ARG(opt, expr) do { \
                                  if (strcmp(opt, argv[i]) == 0) { \
                                      expr; \
                                  } \
                              } while (0)
    for (int i = 0; i < argc; i++) {
        if (strncmp("-", argv[i], 1) != 0) {
            if (i == 0 || file_found) continue;
            file = argv[i];
            file_found = true;
        }
        // Our args
        SBF_ARG("-h",      sbf_usage(argv[0], false));
        SBF_ARG("--help",  sbf_usage(argv[0], false));
        SBF_ARG("-d",      sbf_enable_debug());
        SBF_ARG("--debug", sbf_enable_debug());
    }
    #undef SBF_ARG
    if (!file) sbf_error("no filename provided, see --help for help");
    sbf_debug("opening file named '%s'", file);
    int fd = open(file, O_RDONLY);
    if (fd < 0) sbf_error("couldn't open file: %s", strerror(errno));
    sbf_debug("getting file size");
    struct stat st;
    fstat(fd, &st);
    sbf_debug("file size is %ld", st.st_size);
    sbf_debug("starting execution");
    for (;;) {
        char buf = 0;
        if (pread(fd, &buf, 1, sbf_ip) != 1) break;
        sbf_next_instruction(buf);
        sbf_ip++;
    }
    sbf_debug("cleaning up");
    close(fd);
    return 0;
}
