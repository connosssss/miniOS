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
        print("  ps                   - List running tasks\n");
        print("  ls                   - List files in filesystem\n");
        print("  cat <file>           - Display contents of a file\n");
        print("  touch <file> [text]  - Create or update a file\n");
        print("  rm <file>            - Delete a file from disk\n");
        print("  echo <text>          - Print text to terminal\n");
        print("  clear                - Clear screen\n");
        print("  snake                - Play Snake game\n");
        print("  color <fg> [bg]      - Change text color\n");
    }

    void cmd_ps() {
        static char buf[512];
        int32_t n = syscall(SYS_LIST_PROCS, (uint32_t)buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            print(buf);
        } else {
            print("No processes found.\n");
        }
    }



    char pollchar() {
        return (char)syscall(SYS_POLLCHAR);
    }

    void msleep(uint32_t ms) {
        syscall(SYS_SLEEP, ms);
    }

    void cmd_snake() {

        syscall(SYS_CLEAR);
        print("SNAKE\n");
        print("Controls: W A S D to move, Q to quit\n");
        print("Press any key to start\n");

        while (pollchar() == 0) {
            msleep(10);
        }

        const int BOARD_W = 30;
        const int BOARD_H = 15;
        const int MAX_LEN = 200;

        int snake_x[MAX_LEN];
        int snake_y[MAX_LEN];
        int snake_len = 4;

        for (int i = 0; i < snake_len; i++) {
            snake_x[i] = 10 - i;
            snake_y[i] = 7;
        }

        int dir_x = 1;
        int dir_y = 0;
        uint32_t rng_state = 987654321;

        int food_x = 15;
        int food_y = 8;
        int score = 0;
        bool game_over = false;

        while (!game_over) {
            rng_state = rng_state * 1103515245 + 12345;
            char c = pollchar();

            if (c == 'q') {
                break;
            } 

            else if (c == 'w'&& dir_y != 1) {
                dir_x = 0; dir_y = -1;
            } 
            
            else if (c == 's' && dir_y != -1) {
                dir_x = 0; dir_y = 1;
            } 
            else if (c == 'a' && dir_x != 1) {
                dir_x = -1; dir_y = 0;
            } 
            else if (c == 'd' && dir_x != -1) {
                dir_x = 1; dir_y = 0;
            }

            int next_x = snake_x[0] + dir_x;
            int next_y = snake_y[0] + dir_y;

            if (next_x < 0 || next_x >= BOARD_W || next_y < 0 || next_y >= BOARD_H) {
                game_over = true;
                break;
            }

            for (int i = 0; i < snake_len; i++) {

                if (snake_x[i] == next_x && snake_y[i] == next_y) {
                    game_over = true;
                    break;
                }

            }

            if (game_over) break;

            if (next_x == food_x && next_y == food_y) {

                score += 100;
                if (snake_len < MAX_LEN) snake_len++;
                food_x = (rng_state / 65536) % (BOARD_W - 2) + 1;
                food_y = (rng_state / 131072) % (BOARD_H - 2) + 1;

            }


            for (int i = snake_len - 1; i > 0; i--) {

                snake_x[i] = snake_x[i - 1];
                snake_y[i] = snake_y[i - 1];
            }

            snake_x[0] = next_x;
            snake_y[0] = next_y;

            syscall(SYS_CLEAR);
            print("Score: ");
            char score_str[16];
            int tmp = score, idx = 0;

            if (tmp == 0) score_str[idx++] = '0';

            else {
                char rev[16];
                int r = 0;
                while (tmp > 0) { rev[r++] = '0' + (tmp % 10); tmp /= 10; }
                while (r > 0) score_str[idx++] = rev[--r];
            }

            score_str[idx++] = '\n';
            score_str[idx] = '\0';
            print(score_str);

            print("+");
            for (int x = 0; x < BOARD_W; x++) print("-");
            print("+\n");

            for (int y = 0; y < BOARD_H; y++) {
                print("|");

                for (int x = 0; x < BOARD_W; x++) {
                    if (x == snake_x[0] && y == snake_y[0]) {
                        print("O");
                    } 
                    
                    else {
                        bool is_body = false;
                        for (int i = 1; i < snake_len; i++) {
                            if (snake_x[i] == x && snake_y[i] == y) {
                                is_body = true;
                                break;
                            }
                        }

                        if (is_body) {
                            print("o");
                        } 
                        else if (x == food_x && y == food_y) {
                            print("*");
                        } 
                        else {
                            print(" ");
                        }
                    }
                }
                print("|\n");
            }

            print("+");
            for (int x = 0; x < BOARD_W; x++) print("-");
            print("+\n");
            print("W/A/S/D to move, Q to quit\n");

            msleep(150);
        }

        print("\nFinal Score: ");

        char final_score[16];
        int tmp = score, idx = 0;
        if (tmp == 0) final_score[idx++] = '0';

        else {
            char rev[16];
            int r = 0;
            while (tmp > 0) { rev[r++] = '0' + (tmp % 10); tmp /= 10; }
            while (r > 0) final_score[idx++] = rev[--r];
        }

        final_score[idx++] = '\n';
        final_score[idx] = '\0';
        print(final_score);
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

    void cmd_rm(const char* filename) {
        if (filename[0] == '\0') {
            print("Usage: rm <filename>\n");
            return;
        }
        int32_t res = syscall(SYS_DELETE_FILE, (uint32_t)filename);
        
        if (res == 0) {
            print("Deleted '");
            print(filename);
            print("'\n");
        } 
        
        else {
            print("rm: file not found: ");
            print(filename);
            print("\n");
        }
    }

    int parse_color_name(const char* s) {
        if (streq(s, "black")) return 0;
        if (streq(s, "blue")) return 1;
        if (streq(s, "green")) return 2;
        if (streq(s, "cyan")) return 3;
        if (streq(s, "red")) return 4;
        if (streq(s, "magenta")) return 5;
        if (streq(s, "brown")) return 6;
        if (streq(s, "grey") || streq(s, "gray") || streq(s, "light_grey")) return 7;
        if (streq(s, "dark_grey") || streq(s, "dark_gray")) return 8;
        if (streq(s, "light_blue")) return 9;
        if (streq(s, "light_green")) return 10;
        if (streq(s, "light_cyan")) return 11;
        if (streq(s, "light_red")) return 12;
        if (streq(s, "light_magenta")) return 13;
        if (streq(s, "yellow") || streq(s, "light_brown")) return 14;
        if (streq(s, "white")) return 15;
        return -1;
    }

    void cmd_color(const char* args) {
        static char fg_str[32];
        static char bg_str[32];
        fg_str[0] = '\0';
        bg_str[0] = '\0';

        uint32_t i = 0, j = 0;
        while (args[i] == ' ') i++;

        while (args[i] != '\0' && args[i] != ' ' && j < sizeof(fg_str) - 1) {
            fg_str[j++] = args[i++];
        }
        fg_str[j] = '\0';

        j = 0;
        while (args[i] == ' ') i++;
        while (args[i] != '\0' && args[i] != ' ' && j < sizeof(bg_str) - 1) {
            bg_str[j++] = args[i++];
        }

        bg_str[j] = '\0';

        int fg = parse_color_name(fg_str);
        int bg = (bg_str[0] != '\0') ? parse_color_name(bg_str) : 0;

        if (fg < 0 || bg < 0) {
            print("Usage: color <fg_color> [bg_color]\n");
            print("Available colors:\n");
            print("  black, blue, green, cyan, red, magenta, brown, yellow, white\n");
            print("  light_blue, light_green, light_cyan, light_red, light_magenta, grey, dark_grey\n");
            return;
        }

        syscall(SYS_SET_COLOR, (uint32_t)fg, (uint32_t)bg);
        print("color changed\n");
    }

    void shell_main() {
        syscall(SYS_SET_COLOR, 15, 0); // white on black
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
                    } else if (strstarts(line, "rm ")) {
                        cmd_rm(line + 3);
                    } else if (strstarts(line, "echo ")) {
                        print(line + 5);
                        print("\n");
                    } else if (streq(line, "clear")) {
                        syscall(SYS_CLEAR);
                    } else if (streq(line, "snake")) {
                        cmd_snake();
                    } else if (streq(line, "ps")) {
                        cmd_ps();
                    } else if (streq(line, "color") || strstarts(line, "color ")) {
                        const char* args = strstarts(line, "color ") ? line + 6 : "";
                        cmd_color(args);
                    } 
                    
                    else {
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
            } 
            
            else if (c == '\b' && idx > 0) {
                idx--;
                print("\b \b");
            } 
            
            else if (c >= 32 && c <= 126 && idx < 127) {
                line[idx++] = c;
                char echo_buf[2] = {c, '\0'};
                print(echo_buf);
            }
        }
    }
}

