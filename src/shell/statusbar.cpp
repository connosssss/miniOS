#include "syscall_abi.h"


extern "C" {
    static uint32_t syscall(uint32_t num, uint32_t a = 0, uint32_t b = 0, uint32_t c = 0) {
        uint32_t ret;
        asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(a), "c"(b), "d"(c));
        return ret;
    }

    static void write_at(uint16_t row, uint16_t col, const char* str, uint8_t color) {
        uint32_t pos = (static_cast<uint32_t>(row) << 16) | col;
        syscall(SYS_WRITE_AT, pos, (uint32_t)str, (uint32_t)color);
    }

    static void int_to_str(uint32_t val, char* buf) {
        if (val == 0) {
            buf[0] = '0';
            buf[1] = '\0';
            return;
        }
        char tmp[12];
        int i = 0;
        while (val > 0) {
            tmp[i++] = '0' + (val % 10);
            val /= 10;
        }
        int j = 0;
        while (i > 0) buf[j++] = tmp[--i];
        buf[j] = '\0';
    }


    void statusbar_main() {
        const char spinner[] = "|/-\\";
        uint32_t frame = 0;

        // light cyan text on black background = 0x0B
        const uint8_t BAR_COLOR = 0x0B;
        // light green on black = 0x0A
        const uint8_t ACCENT_COLOR = 0x0A;
        //dark grey on black = 0x08
        const uint8_t DIM_COLOR = 0x08;

        while (1) {
            uint32_t ticks = syscall(SYS_GET_TICKS);
            uint32_t procs = syscall(SYS_GET_PROC_COUNT);
            uint32_t uptime_secs = ticks / 1000;

            char blank[81];
            for (int i = 0; i < 80; i++) blank[i] = ' ';
            blank[80] = '\0';
            write_at(0, 0, blank, BAR_COLOR);

            // spinner
            char header[4];
            header[0] = ' ';
            header[1] = spinner[frame % 4];
            header[2] = ' ';
            header[3] = '\0';
            write_at(0, 0, header, ACCENT_COLOR);

            write_at(0, 3, "\xB3", BAR_COLOR);

            char uptime_buf[24];
            uptime_buf[0] = ' ';
            uptime_buf[1] = 'U';
            uptime_buf[2] = 'p';
            uptime_buf[3] = ':';
            uptime_buf[4] = ' ';
            char secs_str[12];            char blank[81];

            int_to_str(uptime_secs, secs_str);
            int up_pos = 5;
            for (int i = 0; secs_str[i]; i++) uptime_buf[up_pos++] = secs_str[i];
            uptime_buf[up_pos++] = 's';
            uptime_buf[up_pos++] = ' ';
            uptime_buf[up_pos] = '\0';
            write_at(0, 4, uptime_buf, BAR_COLOR);

            int div2_col = 4 + up_pos;
            write_at(0, div2_col, "\xB3", BAR_COLOR);

            int task_col = div2_col + 1;
            char proc_buf[24];
            proc_buf[0] = ' ';
            proc_buf[1] = 'T';
            proc_buf[2] = 'a';
            proc_buf[3] = 's';
            proc_buf[4] = 'k';
            proc_buf[5] = 's';
            proc_buf[6] = ':';
            proc_buf[7] = ' ';
            char proc_str[12];
            int_to_str(procs, proc_str);
            int pr_pos = 8;
            for (int i = 0; proc_str[i]; i++) proc_buf[pr_pos++] = proc_str[i];
            proc_buf[pr_pos++] = ' ';
            proc_buf[pr_pos] = '\0';
            write_at(0, task_col, proc_buf, BAR_COLOR);

            uint32_t hrs = uptime_secs / 3600;
            uint32_t mins = (uptime_secs % 3600) / 60;
            uint32_t secs_r = uptime_secs % 60;
            char time_str[12];
            time_str[0] = ' ';
            time_str[1] = '0' + (hrs / 10);
            time_str[2] = '0' + (hrs % 10);
            time_str[3] = ':';
            time_str[4] = '0' + (mins / 10);
            time_str[5] = '0' + (mins % 10);
            time_str[6] = ':';
            time_str[7] = '0' + (secs_r / 10);
            time_str[8] = '0' + (secs_r % 10);
            time_str[9] = ' ';
            time_str[10] = '\0';
            write_at(0, 70, time_str, DIM_COLOR);

            frame++;
            syscall(SYS_SLEEP, 250); // update 4 times per second
        }
    }

}
