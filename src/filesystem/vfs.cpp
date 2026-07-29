#include "vfs.h"
#include "multiboot.h"
#include "kutil.h"
#include "terminal.h"
#include "serial.h"

namespace {
    struct file_header_t {
        char name[64];
        uint32_t size;
    };

    const uint8_t* initrd_base = nullptr;
    uint32_t file_count = 0;

    constexpr uint32_t MAX_DYNAMIC_FILES = 32;
    constexpr uint32_t MAX_FILE_SIZE = 2048;

    struct dynamic_file_t {
        char name[64];
        char content[MAX_FILE_SIZE];
        uint32_t size;
        bool used;
    };

    dynamic_file_t dynamic_files[MAX_DYNAMIC_FILES] = {};
}

namespace vfs {
    void init(uint32_t mbi_addr) {
        auto* mbi = reinterpret_cast<multiboot_info_t*>(mbi_addr);
        if ((mbi->flags & (1 << 3)) && mbi->mods_count > 0) {
            auto* mod = reinterpret_cast<multiboot_module_t*>(mbi->mods_addr);
            initrd_base = reinterpret_cast<const uint8_t*>(mod->mod_start);
            file_count = *reinterpret_cast<const uint32_t*>(initrd_base);

            terminal::write("vfs initrd loaded: ");
            terminal::write_dec(file_count);
            terminal::write(" file(s).\n");

            serial::write("vfs initrd loaded: ");
            char num[12];
            kutil::dec_to_str(file_count, num);
            serial::write(num);
            serial::write(" file(s).\n");
        } 
        
        else {
            terminal::write("vfs warning: No initrd module loaded by bootloader.\n");
            serial::write("vfs warning: No initrd module loaded by bootloader.\n");
        }
    }

    int32_t create_file(const char* name, const char* content, uint32_t len) {
        if (!name || name[0] == '\0') return -1;

        // Check if file already exists in dynamic storage
        for (uint32_t i = 0; i < MAX_DYNAMIC_FILES; i++) {
            if (dynamic_files[i].used && kutil::strcmp(dynamic_files[i].name, name) == 0) {
                uint32_t write_len = (len >= MAX_FILE_SIZE) ? (MAX_FILE_SIZE - 1) : len;
                kutil::memcpy(dynamic_files[i].content, content, write_len);
                dynamic_files[i].content[write_len] = '\0';
                dynamic_files[i].size = write_len;
                return 0;
            }
        }

        // Find empty slot
        for (uint32_t i = 0; i < MAX_DYNAMIC_FILES; i++) {
            if (!dynamic_files[i].used) {
                uint32_t name_len = kutil::strlen(name);
                if (name_len >= sizeof(dynamic_files[i].name)) name_len = sizeof(dynamic_files[i].name) - 1;
                kutil::memcpy(dynamic_files[i].name, name, name_len);
                dynamic_files[i].name[name_len] = '\0';

                uint32_t write_len = (len >= MAX_FILE_SIZE) ? (MAX_FILE_SIZE - 1) : len;
                kutil::memcpy(dynamic_files[i].content, content, write_len);
                dynamic_files[i].content[write_len] = '\0';
                dynamic_files[i].size = write_len;
                dynamic_files[i].used = true;
                return 0;
            }
        }

        return -1;
    }

    int32_t read_file(const char* name, char* buf, uint32_t max_len) {

        for (uint32_t i = 0; i < MAX_DYNAMIC_FILES; i++) {

            if (dynamic_files[i].used && kutil::strcmp(dynamic_files[i].name, name) == 0) {

                uint32_t read_bytes = (dynamic_files[i].size > max_len) ? max_len : dynamic_files[i].size;
                kutil::memcpy(buf, dynamic_files[i].content, read_bytes);

                return read_bytes;
            }

        }

        if (!initrd_base || file_count == 0) return -1;

        const file_header_t* headers = reinterpret_cast<const file_header_t*>(initrd_base + 4);
        uint32_t data_offset = 4 + file_count * sizeof(file_header_t);

        for (uint32_t i = 0; i < file_count; i++) {
            if (kutil::strcmp(headers[i].name, name) == 0) {
                uint32_t read_bytes = (headers[i].size > max_len) ? max_len : headers[i].size;
                kutil::memcpy(buf, initrd_base + data_offset, read_bytes);
                return read_bytes;
            }
            data_offset += headers[i].size;
        }
        return -1;
    }

    int32_t list_files(char* buf, uint32_t max_len) {

        uint32_t pos = 0;

        if (initrd_base && file_count > 0) {
            const file_header_t* headers = reinterpret_cast<const file_header_t*>(initrd_base + 4);

            for (uint32_t i = 0; i < file_count && pos < max_len; i++) {
                uint32_t len = kutil::strlen(headers[i].name);
                if (pos + len + 1 < max_len) {
                    kutil::memcpy(buf + pos, headers[i].name, len);
                    pos += len;
                    buf[pos++] = '\n';
                }
            }
        }

        for (uint32_t i = 0; i < MAX_DYNAMIC_FILES && pos < max_len; i++) {
            if (dynamic_files[i].used) {
                uint32_t len = kutil::strlen(dynamic_files[i].name);
                if (pos + len + 1 < max_len) {
                    kutil::memcpy(buf + pos, dynamic_files[i].name, len);
                    pos += len;
                    buf[pos++] = '\n';
                }
            }
        }

        if (pos < max_len) buf[pos] = '\0';
        return pos;
    }
}
