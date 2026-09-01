/* ata.c - Implementacion del driver de disco ATA/IDE (PIO, LBA28).
   Lee sectores del canal primario usando el protocolo PIO con polling,
   sin depender de interrupciones. Direccionamiento logico LBA28 sobre el
   master del canal primario (IDE0, iobase 0x1F0). */

#include "drivers/ata.h"
#include "asm.h"

#define ATA_PRIMARY_IO 0x1F0
#define ATA_PRIMARY_CTRL 0x3F6

#define ATA_REG_DATA 0x00
#define ATA_REG_SECTOR_COUNT 0x02
#define ATA_REG_LBA_LOW 0x03
#define ATA_REG_LBA_MID 0x04
#define ATA_REG_LBA_HIGH 0x05
#define ATA_REG_DRIVE_SELECT 0x06
#define ATA_REG_COMMAND 0x07

#define ATA_STATUS_ERR 0x01
#define ATA_STATUS_DRQ 0x08
#define ATA_STATUS_DRDY 0x40
#define ATA_STATUS_BSY 0x80

#define ATA_CMD_READ_SECTORS 0x20

#define ATA_DRV_MASTER_LBA 0xE0

#define ATA_TIMEOUT_LOOPS 1000000

/* Lee el registro de estado (0x1F7) hasta que el disco deje de estar
   ocupado (BSY = 0). Return: 0 ok, negativo si se agota el tiempo. */
static int ata_wait_not_busy(void)
{
    for (uint32_t i = 0; i < ATA_TIMEOUT_LOOPS; i++) {
        uint8_t status = inb(ATA_PRIMARY_IO + ATA_REG_COMMAND);
        if (!(status & ATA_STATUS_BSY)) {
            return 0;
        }
    }
    return -1;
}

/* Espera a que haya datos listos (DRQ) y el disco no este ocupado. */
static int ata_wait_data(void)
{
    for (uint32_t i = 0; i < ATA_TIMEOUT_LOOPS; i++) {
        uint8_t status = inb(ATA_PRIMARY_IO + ATA_REG_COMMAND);
        if (status & ATA_STATUS_ERR) {
            return -2;
        }
        if (!(status & ATA_STATUS_BSY) && (status & ATA_STATUS_DRQ)) {
            return 0;
        }
    }
    return -1;
}

int ata_read_sectors(uint32_t sector, uint8_t *buffer, uint32_t count)
{
    if (count == 0 || count > 256) {
        return -1;
    }

    /* Selecciona el master con modo LBA y los bits 24-27 del LBA. */
    outb(ATA_PRIMARY_IO + ATA_REG_DRIVE_SELECT,
         (uint8_t)(ATA_DRV_MASTER_LBA | ((sector >> 24) & 0x0F)));

    if (ata_wait_not_busy() < 0) {
        return -3;
    }

    if (count == 256) {
        outb(ATA_PRIMARY_IO + ATA_REG_SECTOR_COUNT, 0x00);
    } else {
        outb(ATA_PRIMARY_IO + ATA_REG_SECTOR_COUNT, (uint8_t)count);
    }
    outb(ATA_PRIMARY_IO + ATA_REG_LBA_LOW, (uint8_t)(sector & 0xFF));
    outb(ATA_PRIMARY_IO + ATA_REG_LBA_MID, (uint8_t)((sector >> 8) & 0xFF));
    outb(ATA_PRIMARY_IO + ATA_REG_LBA_HIGH, (uint8_t)((sector >> 16) & 0xFF));

    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

    /* Pequena pausa de E/S para que el disco procese el comando. */
    inb(ATA_PRIMARY_IO + ATA_REG_COMMAND);
    inb(ATA_PRIMARY_IO + ATA_REG_COMMAND);

    uint8_t *dst = buffer;
    for (uint32_t s = 0; s < count; s++) {
        if (ata_wait_data() < 0) {
            return -4;
        }
        /* Lee 512 bytes = 128 dwords desde el registro de datos. */
        for (uint32_t i = 0; i < 128; i++) {
            uint32_t word = inl(ATA_PRIMARY_IO + ATA_REG_DATA);
            dst[0] = (uint8_t)(word & 0xFF);
            dst[1] = (uint8_t)((word >> 8) & 0xFF);
            dst[2] = (uint8_t)((word >> 16) & 0xFF);
            dst[3] = (uint8_t)((word >> 24) & 0xFF);
            dst += 4;
        }
    }

    return 0;
}
