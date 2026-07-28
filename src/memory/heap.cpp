#include "heap.h"
#include "pmm.h"
#include "terminal.h"
#include "serial.h"

namespace {
    struct chunk_header_t {
        size_t size;
        bool is_free;
        chunk_header_t* next;
    };

    constexpr size_t PAGE_SIZE = 4096;
    chunk_header_t* free_list_head = nullptr;
}

namespace heap {
    void init() {
        free_list_head = nullptr;
        terminal::write("heap initialized.\n");
        serial::write("heap initialized.\n");
    }

    void* kmalloc(size_t size) {
        if (size == 0 || size > PAGE_SIZE - sizeof(chunk_header_t)) return nullptr;

        chunk_header_t* prev = nullptr;
        chunk_header_t* cur = free_list_head;

        while (cur) {
            if (cur->is_free && cur->size >= size) {
                cur->is_free = false;
                return reinterpret_cast<void*>(cur + 1);
            }
            prev = cur;
            cur = cur->next;
        }

        uint32_t frame = pmm::alloc_frame();
        if (!frame) return nullptr;

        cur = reinterpret_cast<chunk_header_t*>(frame);
        cur->size = PAGE_SIZE - sizeof(chunk_header_t);
        cur->is_free = false;
        cur->next = nullptr;

        if (prev) prev->next = cur;
        else free_list_head = cur;

        return reinterpret_cast<void*>(cur + 1);
    }




    void kfree(void* ptr) {
        if (!ptr) return;

        auto* header = reinterpret_cast<chunk_header_t*>(ptr) - 1;
        header->is_free = true;
    }



    bool self_test() {

        void* p1 = kmalloc(128);
        void* p2 = kmalloc(256);

        kfree(p1);
        void* p3 = kmalloc(64); 

        bool passed = (p1 != nullptr && p2 != nullptr && p3 == p1);

        kfree(p2);
        kfree(p3);

        return passed;
    }
}
