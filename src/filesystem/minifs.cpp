#include "minifs.h"
#include "ata.h"
#include "kutil.h"
#include "terminal.h"
#include "serial.h"

namespace {
    minifs::superblock_t sb;
    bool fs_ready = false;

    // num of file entries per sector (512 / 128 = 4)
    constexpr uint32_t ENTRIES_PER_SECTOR = ata::SECTOR_SIZE / sizeof(minifs::file_entry_t);

    // max file entries 32 sectors × 4 entries = 128
    constexpr uint32_t FILE_TABLE_SECTORS = 32;
    constexpr uint32_t MAX_FILES = FILE_TABLE_SECTORS * ENTRIES_PER_SECTOR;

    // bitmap helpers
    // each bitmap sector tracks 512*8 = 4096 blocks
    constexpr uint32_t BITS_PER_SECTOR = ata::SECTOR_SIZE * 8;

    uint8_t bitmap_buf[ata::SECTOR_SIZE];
    uint8_t sector_buf[ata::SECTOR_SIZE];

    bool bitmap_test(uint32_t block) {

        uint32_t bitmap_sector = sb.bitmap_start + (block / BITS_PER_SECTOR);
        uint32_t bit_offset = block % BITS_PER_SECTOR;
        uint32_t byte_offset = bit_offset / 8;
        uint32_t bit_index = bit_offset % 8;


        if (!ata::read_sectors(bitmap_sector, 1, bitmap_buf)) return true; 
        return bitmap_buf[byte_offset] & (1 << bit_index);
    }



    void bitmap_set(uint32_t block, bool used) {
        uint32_t bitmap_sector = sb.bitmap_start + (block / BITS_PER_SECTOR);
        uint32_t bit_offset = block % BITS_PER_SECTOR;
        uint32_t byte_offset = bit_offset / 8;
        uint32_t bit_index = bit_offset % 8;

        ata::read_sectors(bitmap_sector, 1, bitmap_buf);
        if (used) {
            bitmap_buf[byte_offset] |= (1 << bit_index);
        } 
        
        else {
            bitmap_buf[byte_offset] &= ~(1 << bit_index);
        }
        ata::write_sectors(bitmap_sector, 1, bitmap_buf);
    }

    // find count of contiguous free blocks starting from the data area
    uint32_t find_free_blocks(uint32_t count) {
        if (count == 0) return sb.data_start;

        uint32_t run_start = sb.data_start;
        uint32_t run_len = 0;

        for (uint32_t b = sb.data_start; b < sb.total_blocks; b++) {

            if (!bitmap_test(b)) {
                if (run_len == 0) run_start = b;
                run_len++;

                if (run_len >= count) return run_start;
            } 
            
            else {
                run_len = 0;
            }
        }

        return 0xFFFFFFFF; // not enough space
    }

    bool read_file_entry(uint32_t index, minifs::file_entry_t* entry) {

        uint32_t sector = sb.file_table_start + (index / ENTRIES_PER_SECTOR);
        uint32_t offset = (index % ENTRIES_PER_SECTOR) * sizeof(minifs::file_entry_t);

        if (!ata::read_sectors(sector, 1, sector_buf)) return false;
        kutil::memcpy(entry, sector_buf + offset, sizeof(minifs::file_entry_t));

        return true;
    }

    bool write_file_entry(uint32_t index, const minifs::file_entry_t* entry) {

        uint32_t sector = sb.file_table_start + (index / ENTRIES_PER_SECTOR);
        uint32_t offset = (index % ENTRIES_PER_SECTOR) * sizeof(minifs::file_entry_t);


        if (!ata::read_sectors(sector, 1, sector_buf)) return false;
        kutil::memcpy(sector_buf + offset, entry, sizeof(minifs::file_entry_t));
        return ata::write_sectors(sector, 1, sector_buf);
    }



    bool write_superblock() {
        return ata::write_sectors(0, 1, &sb);
    }



    uint32_t find_file(const char* name) {
        minifs::file_entry_t entry;

        for (uint32_t i = 0; i < MAX_FILES; i++) {
            if (!read_file_entry(i, &entry)) continue;
            if (entry.flags == 1 && kutil::strcmp(entry.name, name) == 0) {
                return i;
            }
        }
        return MAX_FILES;
    }

    uint32_t find_free_entry() {
        minifs::file_entry_t entry;

        for (uint32_t i = 0; i < MAX_FILES; i++) {
            if (!read_file_entry(i, &entry)) continue;
            if (entry.flags == 0) return i;
        }
        return MAX_FILES;
    }
}

namespace minifs {
    bool format() {
        uint32_t total = ata::sector_count();
        if (total == 0) return false;

        serial::write("[minifs] Formatting disk...\n");
        terminal::write("[minifs] Formatting disk...\n");



        uint32_t bitmap_sectors = ((total + BITS_PER_SECTOR - 1) / BITS_PER_SECTOR);
        uint32_t ft_start = 1 + bitmap_sectors;
        uint32_t d_start = ft_start + FILE_TABLE_SECTORS;

        kutil::memset(&sb, 0, sizeof(sb));
        sb.magic = MINIFS_MAGIC;
        sb.version = 1;
        sb.total_blocks = total;
        sb.block_size = ata::SECTOR_SIZE;
        sb.free_blocks = total - d_start; 
        sb.bitmap_start = 1;
        sb.bitmap_count = bitmap_sectors;
        sb.file_table_start = ft_start;
        sb.file_table_count = FILE_TABLE_SECTORS;
        sb.data_start = d_start;

        if (!ata::write_sectors(0, 1, &sb)) return false;

        kutil::memset(sector_buf, 0, ata::SECTOR_SIZE);
        for (uint32_t i = 0; i < bitmap_sectors; i++) {
            if (!ata::write_sectors(sb.bitmap_start + i, 1, sector_buf)) return false;
        }

        for (uint32_t b = 0; b < d_start; b++) {
            bitmap_set(b, true);
        }

        kutil::memset(sector_buf, 0, ata::SECTOR_SIZE);
        for (uint32_t i = 0; i < FILE_TABLE_SECTORS; i++) {
            if (!ata::write_sectors(ft_start + i, 1, sector_buf)) return false;
        }

        sb.free_blocks = total - d_start;
        write_superblock();

        serial::write("[minifs] Format complete.\n");
        terminal::write("[minifs] Format complete.\n");

        fs_ready = true;
        return true;
    }

    bool init() {
        if (ata::sector_count() == 0) {
            serial::write("[minifs] No disk available.\n");
            terminal::write("[minifs] No disk.\n");
            return false;
        }

        if (!ata::read_sectors(0, 1, &sb)) {
            serial::write("[minifs] Failed to read superblock.\n");
            return false;
        }

        if (sb.magic != MINIFS_MAGIC) {
            serial::write("[minifs] No filesystem found, auto-formatting.\n");
            terminal::write("[minifs] Auto-formatting disk...\n");
            return format();
        }

        fs_ready = true;

        serial::write("[minifs] Filesystem loaded: ");
        char num[12];
        kutil::dec_to_str(sb.free_blocks, num);
        serial::write(num);
        serial::write(" free blocks.\n");

        terminal::write("[minifs] Disk FS ready: ");
        terminal::write_dec(sb.free_blocks);
        terminal::write(" free blocks.\n");

        return true;
    }

    int32_t create(const char* name, const char* data, uint32_t len) {
        if (!fs_ready || !name || name[0] == '\0') return -1;

        // wipe a file if it already exists
        uint32_t existing = find_file(name);
        if (existing < MAX_FILES) {
            remove(name);
        }

        uint32_t sectors_needed = (len + ata::SECTOR_SIZE - 1) / ata::SECTOR_SIZE;
        if (sectors_needed == 0) sectors_needed = 0; // empty file is OK

  
        uint32_t entry_idx = find_free_entry();
        if (entry_idx >= MAX_FILES) {
            serial::write("[minifs] File table full.\n");
            return -1;
        }

        uint32_t start_block = 0;
        if (sectors_needed > 0) {
            start_block = find_free_blocks(sectors_needed);
            if (start_block == 0xFFFFFFFF) {
                serial::write("[minifs] Disk full.\n");
                return -1;
            }


            for (uint32_t i = 0; i < sectors_needed; i++) {

                kutil::memset(sector_buf, 0, ata::SECTOR_SIZE);
                uint32_t offset = i * ata::SECTOR_SIZE;
                uint32_t chunk = len - offset;

                if (chunk > ata::SECTOR_SIZE) chunk = ata::SECTOR_SIZE;

                kutil::memcpy(sector_buf, data + offset, chunk);

                if (!ata::write_sectors(start_block + i, 1, sector_buf)) {
                    return -1;
                }
            }

            for (uint32_t i = 0; i < sectors_needed; i++) {
                bitmap_set(start_block + i, true);
            }
        }

        file_entry_t entry;
        kutil::memset(&entry, 0, sizeof(entry));
        uint32_t name_len = kutil::strlen(name);
        if (name_len >= sizeof(entry.name)) name_len = sizeof(entry.name) - 1;
        kutil::memcpy(entry.name, name, name_len);
        entry.name[name_len] = '\0';
        entry.size = len;
        entry.start_block = start_block;
        entry.block_count = sectors_needed;
        entry.flags = 1; 

        if (!write_file_entry(entry_idx, &entry)) return -1;

        sb.free_blocks -= sectors_needed;
        write_superblock();

        return 0;
    }

    int32_t read(const char* name, char* buf, uint32_t max_len) {
        if (!fs_ready || !name) return -1;

        uint32_t idx = find_file(name);
        if (idx >= MAX_FILES) return -1;

        file_entry_t entry;
        if (!read_file_entry(idx, &entry)) return -1;

        uint32_t to_read = entry.size;
        if (to_read > max_len) to_read = max_len;

        uint32_t sectors = (to_read + ata::SECTOR_SIZE - 1) / ata::SECTOR_SIZE;
        uint32_t bytes_read = 0;

        for (uint32_t i = 0; i < sectors; i++) {
            if (!ata::read_sectors(entry.start_block + i, 1, sector_buf)) return -1;

            uint32_t chunk = to_read - bytes_read;
            if (chunk > ata::SECTOR_SIZE) chunk = ata::SECTOR_SIZE;
            kutil::memcpy(buf + bytes_read, sector_buf, chunk);
            bytes_read += chunk;
        }

        return static_cast<int32_t>(bytes_read);
    }

    int32_t remove(const char* name) {
        if (!fs_ready || !name) return -1;

        uint32_t idx = find_file(name);
        if (idx >= MAX_FILES) return -1;

        file_entry_t entry;
        if (!read_file_entry(idx, &entry)) return -1;

        for (uint32_t i = 0; i < entry.block_count; i++) {
            bitmap_set(entry.start_block + i, false);
        }

        sb.free_blocks += entry.block_count;
        write_superblock();

        kutil::memset(&entry, 0, sizeof(entry));
        write_file_entry(idx, &entry);

        return 0;
    }

    int32_t list(char* buf, uint32_t max_len) {
        if (!fs_ready) return 0;

        uint32_t pos = 0;
        file_entry_t entry;

        for (uint32_t i = 0; i < MAX_FILES && pos < max_len; i++) {
            if (!read_file_entry(i, &entry)) continue;
            if (entry.flags != 1) continue;

            uint32_t name_len = kutil::strlen(entry.name);
            if (pos + name_len + 1 < max_len) {
                kutil::memcpy(buf + pos, entry.name, name_len);
                pos += name_len;
                buf[pos++] = '\n';
            }
        }

        if (pos < max_len) buf[pos] = '\0';

        return static_cast<int32_t>(pos);
    }
}
