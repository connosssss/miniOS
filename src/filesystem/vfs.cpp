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
}

namespace vfs {
    void init(uint32_t mbi_addr) {
        auto* mbi = reinterpret_cast<multiboot_info_t*>(mbi_addr);
        if ((mbi->flags & (1 << 3)) && mbi->mods_count > 0) {
            auto* mod = reinterpret_cast<multiboot_module_t*>(mbi->mods_addr);
            initrd_base = reinterpret_cast<const uint8_t*>(mod->mod_start);
            file_count = *reinterpret_cast<const uint32_t*>(initrd_base);

            terminal::write("[vfs] initrd loaded: ");
            terminal::write_dec(file_count);
            terminal::write(" file(s).\n");

            serial::write("[vfs] initrd loaded: ");
            char num[12];
            kutil::dec_to_str(file_count, num);
            serial::write(num);
            serial::write(" file(s).\n");
        } else {
            terminal::write("[vfs] WARNING: No initrd module loaded by bootloader.\n");
            serial::write("[vfs] WARNING: No initrd module loaded by bootloader.\n");
        }
    }

    int32_t read_file(const char* name, char* buf, uint32_t max_len) {
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
        if (!initrd_base) return 0;
        const file_header_t* headers = reinterpret_cast<const file_header_t*>(initrd_base + 4);
        uint32_t pos = 0;
        for (uint32_t i = 0; i < file_count && pos < max_len; i++) {
            uint32_t len = kutil::strlen(headers[i].name);
            if (pos + len + 1 < max_len) {
                kutil::memcpy(buf + pos, headers[i].name, len);
                pos += len;
                buf[pos++] = '\n';
            }
        }
        
        if (pos < max_len) buf[pos] = '\0';
        return pos;
    }
}
