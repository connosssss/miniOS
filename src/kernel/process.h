#pragma once
#include <stdint.h>
#include <stddef.h>
#include "idt.h"

enum ProcessState {
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_TERMINATED
};

struct process_t {

    uint32_t pid;
    ProcessState state;
    uint32_t page_directory_phys; // physical address for CR3
    uint32_t kernel_stack_bottom;
    uint32_t kernel_stack_top; 
    registers_t* regs; // saved CPU register frame pointer on kernel stack
    process_t* next;
};




namespace process {
    void init();
    process_t* create(void (*entry_point)(), bool is_user);
    registers_t* schedule(registers_t* current_regs);
    process_t* current();
    void yield();
    bool is_multitasking_active();
    void start_multitasking();
}
