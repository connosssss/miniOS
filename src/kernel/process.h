#pragma once
#include <stdint.h>
#include <stddef.h>
#include "idt.h"

enum ProcessState {
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_SLEEPING,
    PROCESS_TERMINATED
};

struct process_t {

    uint32_t pid;
    ProcessState state;
    char name[16];
    bool is_user;
    uint32_t page_directory_phys; // physical address for CR3
    uint32_t kernel_stack_bottom;
    uint32_t kernel_stack_top; 
    registers_t* regs; // saved CPU register frame pointer on kernel stack
    uint64_t sleep_until; // tick count to wake at (0 = not sleeping)
    process_t* next;
};




namespace process {
    void init();
    process_t* create(void (*entry_point)(), bool is_user, const char* name = "process");
    registers_t* schedule(registers_t* current_regs);
    process_t* current();
    void yield();
    bool is_multitasking_active();
    void start_multitasking();
    uint32_t count();
    uint32_t list_procs(char* buf, uint32_t size);
    void sleep_current(uint32_t ms, uint64_t current_ticks);
}
