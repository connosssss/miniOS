#include "syscall_abi.h"

extern "C" {
    uint32_t syscall(uint32_t num, uint32_t a = 0, uint32_t b = 0, uint32_t c = 0) {
        uint32_t ret;
        asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(a), "c"(b), "d"(c));
        return ret;
    }

    void print(const char* s) {
        syscall(SYS_WRITE, (uint32_t)s);
    }

    char getchar() {
        return (char)syscall(SYS_GETCHAR);
    }

    bool streq(const char* a, const char* b) {
        while (*a && *b) {
            if (*a != *b) return false;
            a++;
            b++;
        }
        return *a == *b;
    }

    bool strstarts(const char* str, const char* prefix) {
        while (*prefix) {
            if (*str != *prefix) return false;
            str++;
            prefix++;
        }
        return true;
    }

    void cmd_help() {
        print("Available commands:\n");
        print("  help                 - Show this help message\n");
        print("  ls                   - List files in filesystem\n");
        print("  cat <file>           - Display contents of a file\n");
        print("  touch <file> [text]  - Create or update a file\n");
        print("  echo <text>          - Print text to terminal\n");
        print("  clear                - Clear screen\n");
    }

    void cmd_ls() {
        static char buf[512];
        int32_t n = syscall(SYS_LIST_FILES, (uint32_t)buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            print(buf);
        } else {
            print("No files found.\n");
        }
    }

    void cmd_cat(const char* filename) {
        static char buf[2048];
        int32_t n = syscall(SYS_READ_FILE, (uint32_t)filename, (uint32_t)buf, sizeof(buf) - 1);
        if (n < 0) {
            print("cat: file not found: ");
            print(filename);
            print("\n");
            return;
        }
        buf[n] = '\0';
        print(buf);
        print("\n");
    }

    void cmd_touch(const char* args) {
        static char name[64];
        uint32_t i = 0;
        
        while (args[i] != '\0' && args[i] != ' ' && i < sizeof(name) - 1) {
            name[i] = args[i];
            i++;
        }
        name[i] = '\0';

        if (name[0] == '\0') {
            print("Usage: touch <filename> [content]\n");
            return;
        }

        const char* text = "";
        if (args[i] == ' ') {
            text = args + i + 1;
        }

        uint32_t len = 0;
        while (text[len] != '\0') len++;

        int32_t res = syscall(SYS_CREATE_FILE, (uint32_t)name, (uint32_t)text, len);
        if (res == 0) {
            print("Created file '");
            print(name);
            print("'\n");
        } else {
            print("Failed to create file.\n");
        }
    }

    void shell_main() {
        print("miniOS shell (ring 3). Type 'help' for a list of commands.\n> ");
        char line[128];
        uint32_t idx = 0;

        while (true) {
            char c = getchar();
            if (c == '\n') {
                line[idx] = '\0';
                print("\n");

                if (line[0] != '\0') {
                    if (streq(line, "help")) {
                        cmd_help();
                    } else if (streq(line, "ls")) {
                        cmd_ls();
                    } else if (strstarts(line, "cat ")) {
                        cmd_cat(line + 4);
                    } else if (strstarts(line, "touch ")) {
                        cmd_touch(line + 6);
                    } else if (strstarts(line, "echo ")) {
                        print(line + 5);
                        print("\n");
                    } else if (streq(line, "clear")) {
                        syscall(SYS_CLEAR);
                    } else {
                        // Try reading line as a filename directly
                        static char buf[1024];
                        int32_t n = syscall(SYS_READ_FILE, (uint32_t)line, (uint32_t)buf, sizeof(buf) - 1);
                        if (n >= 0) {
                            buf[n] = '\0';
                            print(buf);
                            print("\n");
                        } else {
                            print("Unknown command or file: ");
                            print(line);
                            print("\n");
                        }
                    }
                }

                print("> ");
                idx = 0;
            } else if (c == '\b' && idx > 0) {
                idx--;
                print("\b \b");
            } else if (c >= 32 && c <= 126 && idx < 127) {
                line[idx++] = c;
                char echo_buf[2] = {c, '\0'};
                print(echo_buf);
            }
        }
    }
}

