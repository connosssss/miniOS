#include "ata.h"
#include "io.h"
#include "terminal.h"
#include "serial.h"
#include "kutil.h"

namespace {
    constexpr uint16_t ATA_DATA        = 0x1F0;
    constexpr uint16_t ATA_ERROR       = 0x1F1;
    constexpr uint16_t ATA_SECT_COUNT  = 0x1F2;
    constexpr uint16_t ATA_LBA_LO     = 0x1F3;
    constexpr uint16_t ATA_LBA_MID    = 0x1F4;
    constexpr uint16_t ATA_LBA_HI     = 0x1F5;
    constexpr uint16_t ATA_DRIVE_HEAD = 0x1F6;
    constexpr uint16_t ATA_STATUS     = 0x1F7;
    constexpr uint16_t ATA_COMMAND    = 0x1F7;
    constexpr uint16_t ATA_ALT_STATUS = 0x3F6;

    constexpr uint8_t ATA_SR_BSY  = 0x80;
    constexpr uint8_t ATA_SR_DRDY = 0x40;
    constexpr uint8_t ATA_SR_DRQ  = 0x08;
    constexpr uint8_t ATA_SR_ERR  = 0x01;

    constexpr uint8_t ATA_CMD_READ_PIO  = 0x20;
    constexpr uint8_t ATA_CMD_WRITE_PIO = 0x30;
    constexpr uint8_t ATA_CMD_IDENTIFY  = 0xEC;
    constexpr uint8_t ATA_CMD_FLUSH     = 0xE7;

    uint32_t drive_sectors = 0;
    bool drive_present = false;

    void ata_wait_bsy() {
        while (inb(ATA_STATUS) & ATA_SR_BSY) {
            // spin
        }
    }

    // Wait for DRQ to set (data ready)
    void ata_wait_drq() {
        while (!(inb(ATA_STATUS) & ATA_SR_DRQ)) {
            // spin
        }
    }

    // 400ns delay by reading alternate status 4 times
    void ata_delay() {
        inb(ATA_ALT_STATUS);
        inb(ATA_ALT_STATUS);
        inb(ATA_ALT_STATUS);
        inb(ATA_ALT_STATUS);
    }

    void ata_read_sector_data(uint16_t* buf) {

        for (int i = 0; i < 256; i++) {
            buf[i] = inw(ATA_DATA);
        }
    }

    void ata_write_sector_data(const uint16_t* buf) {
        for (int i = 0; i < 256; i++) {
            outw(ATA_DATA, buf[i]);
        }
    }

    void ata_select_lba28(uint32_t lba, uint8_t count) {
        outb(ATA_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F)); // master, LBA mode

        outb(ATA_SECT_COUNT, count);
        outb(ATA_LBA_LO,  lba & 0xFF);
        outb(ATA_LBA_MID, (lba >> 8) & 0xFF);
        outb(ATA_LBA_HI,  (lba >> 16) & 0xFF);
    }
}

namespace ata {
    bool init() {
        outb(ATA_DRIVE_HEAD, 0xA0);
        ata_delay();

        outb(ATA_SECT_COUNT, 0);
        outb(ATA_LBA_LO, 0);
        outb(ATA_LBA_MID, 0);
        outb(ATA_LBA_HI, 0);

        outb(ATA_COMMAND, ATA_CMD_IDENTIFY);
        ata_delay();

        uint8_t status = inb(ATA_STATUS);
        if (status == 0) {
            serial::write("[ata] No drive on primary master.\n");
            terminal::write("[ata] No drive found.\n");
            drive_present = false;
            return false;
        }

        ata_wait_bsy();

        if (inb(ATA_LBA_MID) != 0 || inb(ATA_LBA_HI) != 0) {
            serial::write("[ata] Non-ATA drive detected, skipping.\n");
            drive_present = false;
            return false;
        }

        while (true) {
            status = inb(ATA_STATUS);

            if (status & ATA_SR_ERR) {
                serial::write("[ata] IDENTIFY error.\n");
                drive_present = false;
                return false;
            }

            if (status & ATA_SR_DRQ) break;
        }



        uint16_t identify[256];
        ata_read_sector_data(identify);

        drive_sectors = (uint32_t)identify[61] << 16 | identify[60];
        drive_present = true;

        serial::write("ata Drive found: ");
        char num[12];
        kutil::dec_to_str(drive_sectors, num);
        serial::write(num);
        serial::write(" sectors (");
        kutil::dec_to_str((drive_sectors * 512) / 1024, num);
        serial::write(num);
        serial::write(" KB)\n");

        terminal::write("ata Drive: ");
        terminal::write_dec((drive_sectors * 512) / 1024);
        terminal::write(" KB\n");

        return true;
    }

    bool read_sectors(uint32_t lba, uint32_t count, void* buffer) {
        if (!drive_present || count == 0) return false;

        uint16_t* buf = static_cast<uint16_t*>(buffer);

        for (uint32_t i = 0; i < count; i++) {
            ata_wait_bsy();
            ata_select_lba28(lba + i, 1);
            outb(ATA_COMMAND, ATA_CMD_READ_PIO);
            ata_delay();
            ata_wait_bsy();

            uint8_t status = inb(ATA_STATUS);
            if (status & ATA_SR_ERR) return false;

            ata_wait_drq();
            ata_read_sector_data(buf + i * 256);
        }


        return true;
    }

    bool write_sectors(uint32_t lba, uint32_t count, const void* buffer) {
        if (!drive_present || count == 0) return false;

        const uint16_t* buf = static_cast<const uint16_t*>(buffer);

        for (uint32_t i = 0; i < count; i++) {
            ata_wait_bsy();
            ata_select_lba28(lba + i, 1);
            outb(ATA_COMMAND, ATA_CMD_WRITE_PIO);
            ata_delay();
            ata_wait_bsy();
            ata_wait_drq();

            ata_write_sector_data(buf + i * 256);

            outb(ATA_COMMAND, ATA_CMD_FLUSH);
            ata_wait_bsy();
        }

        return true;
    }

    uint32_t sector_count() {    // 400ns delay by reading alternate status 4 times

        return drive_sectors;
    }
}



