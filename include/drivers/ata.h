/* ata.h - Driver de disco ATA/IDE via PIO (LBA28).
   Proporciona lectura de sectores del canal primario mediante polling,
   suficiente para que el filesystem FAT32 acceda al disco virtual que
   QEMU adjunta a la maquina (master del canal primario). */

#pragma once

#include "types.h"

/** ata_read_sectors - Lee @count sectores de 512 bytes desde @lba.
    Usa el modo PIO (polling) contra el master del canal primario ATA.
    @sector: LBA inicial (sector logico)
    @buffer: destino de los datos (debe poder alojar count*512 bytes)
    @count:  numero de sectores consecutivos a leer (1-256)
    Return:  0 en exito, negativo en error. */
int ata_read_sectors(uint32_t sector, uint8_t *buffer, uint32_t count);
