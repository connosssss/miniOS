#pragma once
#include <stdint.h>

namespace minifs {
    // Initialize the filesystem. Reads the superblock from disk.
    // If the disk is unformatted (bad magic), automatically formats it.
    // Returns true if the filesystem is ready to use.
    bool init();

    // Format the disk with a fresh miniFS.
    bool format();

    // Create or overwrite a file with the given content.
    // Returns 0 on success, -1 on error.
    int32_t create(const char* name, const char* data, uint32_t len);

    // Read a file into buf. Returns bytes read, or -1 if not found.
    int32_t read(const char* name, char* buf, uint32_t max_len);

    // Delete a file. Returns 0 on success, -1 if not found.
    int32_t remove(const char* name);

    // List all files into buf (newline-separated). Returns bytes written.
    int32_t list(char* buf, uint32_t max_len);

    // On-disk structures (public for debugging)
    constexpr uint32_t MINIFS_MAGIC = 0x4D494E49; // "MINI"

    struct superblock_t {
        uint32_t magic;
        uint32_t version;
        uint32_t total_blocks;
        uint32_t block_size;       // always 512
        uint32_t free_blocks;
        uint32_t file_table_start; 
        uint32_t file_table_count; 
        uint32_t data_start; 
        uint32_t bitmap_start; 
        uint32_t bitmap_count; 
        uint8_t padding[472];
    } __attribute__((packed));

    struct file_entry_t {
        char name[64];
        uint32_t size;
        uint32_t start_block;      
        uint32_t block_count;
        uint32_t flags; 
        uint8_t padding[48];
    } __attribute__((packed));
}
