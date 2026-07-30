#include "syscall_abi.h"


extern "C" {

    static uint32_t syscall(uint32_t num, uint32_t a = 0, uint32_t b = 0, uint32_t c = 0) {
        uint32_t ret;
        asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(a), "c"(b), "d"(c));
        return ret;
    }

    static void print(const char* s) {
        syscall(SYS_WRITE_SERIAL, (uint32_t)s);
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

    void sysmon_main() {
        uint32_t heartbeat = 0;
        syscall(SYS_SLEEP, 2000);

        print("system monitor started\n");

        while (1) {
            syscall(SYS_SLEEP, 5000); 
            heartbeat++;

            uint32_t ticks = syscall(SYS_GET_TICKS);
            uint32_t procs = syscall(SYS_GET_PROC_COUNT);
            uint32_t uptime_secs = ticks / 1000;

            char num_buf[12];

            print("uptime=");
            int_to_str(uptime_secs, num_buf);
            print(num_buf);
            print("s");

            print(" | tasks=");
            int_to_str(procs, num_buf);
            print(num_buf);

            print(" | ticks=");
            int_to_str(ticks, num_buf);
            print(num_buf);

            print("\n");
        }
    }
}
