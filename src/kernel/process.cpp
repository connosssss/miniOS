#include "process.h"
#include "paging.h"
#include "pmm.h"
#include "heap.h"
#include "gdt.h"
#include "terminal.h"
#include "serial.h"
#include "kutil.h"

extern "C" char _user_start[];
extern "C" char _user_end[];

namespace {
    process_t* process_head = nullptr;
    process_t* current_proc = nullptr;
    uint32_t next_pid = 1;
    bool multitasking_enabled = false;
}


namespace process {
    void init() {
        process_head = nullptr;
        current_proc = nullptr;
        next_pid = 1;
        multitasking_enabled = false;
    }


    bool is_multitasking_active() {
        return multitasking_enabled;
    }


    process_t* current() {
        return current_proc;
    }


    process_t* create(void (*entry_point)(), bool is_user) {
        process_t* proc = reinterpret_cast<process_t*>(heap::kmalloc(sizeof(process_t)));
        proc->pid = next_pid++;
        proc->state = PROCESS_READY;
        proc->next = nullptr;

        //allocate kernel stack
        uint32_t kstack_size = 4096;
        proc->kernel_stack_bottom = pmm::alloc_frame();
        proc->kernel_stack_top = proc->kernel_stack_bottom + kstack_size;

        if (is_user) {
            proc->page_directory_phys = paging::create_user_page_directory();

            //allocate user stack
            uint32_t ustack_size = 4096;
            uint32_t ustack_bottom = pmm::alloc_frame();
            uint32_t ustack_top = ustack_bottom + ustack_size;
            uint32_t user_size = reinterpret_cast<uint32_t>(_user_end) - reinterpret_cast<uint32_t>(_user_start);


            paging::set_user_accessible_in_dir(proc->page_directory_phys, reinterpret_cast<uint32_t>(_user_start), user_size);
            paging::set_user_accessible_in_dir(proc->page_directory_phys, ustack_bottom, ustack_size);
            registers_t* regs = reinterpret_cast<registers_t*>(proc->kernel_stack_top - sizeof(registers_t));
            proc->regs = regs;


            regs->ds = 0x23;
            regs->edi = 0; regs->esi = 0; regs->ebp = 0; regs->esp = 0;
            regs->ebx = 0; regs->edx = 0; regs->ecx = 0; regs->eax = 0;
            regs->int_no = 0; regs->err_code = 0;
            regs->eip = reinterpret_cast<uint32_t>(entry_point);
            regs->cs = 0x1B;
            regs->eflags = 0x202;
            regs->useresp = ustack_top;
            regs->ss = 0x23;
        } 
        
        else {
            proc->page_directory_phys = paging::get_kernel_page_directory();

            uint32_t ring0_frame_size = sizeof(registers_t) - 8;
            registers_t* regs = reinterpret_cast<registers_t*>(proc->kernel_stack_top - ring0_frame_size);
            proc->regs = regs;

            regs->ds = 0x10;
            regs->edi = 0; regs->esi = 0; regs->ebp = 0; regs->esp = 0;
            regs->ebx = 0; regs->edx = 0; regs->ecx = 0; regs->eax = 0;
            regs->int_no = 0; regs->err_code = 0;
            regs->eip = reinterpret_cast<uint32_t>(entry_point);

            regs->cs = 0x08;
            regs->eflags = 0x202;
        }


        // circular linked list for round robin scheduling (not priority based)
        if (!process_head) {
            process_head = proc;
            proc->next = proc;
        } 
        
        else {
            process_t* curr = process_head;
            while (curr->next != process_head) {
                curr = curr->next;
            }
            curr->next = proc;
            proc->next = process_head;
        }

        return proc;
    }

    registers_t* schedule(registers_t* current_regs) {
        if (!multitasking_enabled || !current_proc) {
            return current_regs;
        }

        // save active context
        current_proc->regs = current_regs;
        if (current_proc->state == PROCESS_RUNNING) {
            current_proc->state = PROCESS_READY;
        }

        // find the next ready task
        process_t* next_proc = current_proc->next;
        
        while (next_proc->state != PROCESS_READY && next_proc != current_proc) {
            next_proc = next_proc->next;
        }

        current_proc = next_proc;
        current_proc->state = PROCESS_RUNNING;

        paging::switch_page_directory(current_proc->page_directory_phys);
        gdt::set_kernel_stack(current_proc->kernel_stack_top);

        return current_proc->regs;
    }


    void yield() {
        if (multitasking_enabled) {
            asm volatile("int $0x80" : : "a"(11));
        }
    }





extern "C" void restore_registers(registers_t* regs);
    void start_multitasking() {
        if (!process_head) return;
        current_proc = process_head;
        current_proc->state = PROCESS_RUNNING;
        multitasking_enabled = true;

        paging::switch_page_directory(current_proc->page_directory_phys);
        gdt::set_kernel_stack(current_proc->kernel_stack_top);

        restore_registers(current_proc->regs);
    }

}


