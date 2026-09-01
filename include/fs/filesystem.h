/* filesystem.h - Estructuras de bajo nivel de los sistemas de archivos.
   Define el sector de arranque (boot sector) y las entradas de directorio
   del filesystem FAT32 tal como se almacenan en disco. */

#pragma once

#include "types.h"

/* Sector de arranque de FAT32 (formato BPB extendido). */
typedef struct {
    uint8_t boot_jump[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_dir_entries; /* 0 en FAT32 */
    uint16_t total_sectors;    /* 0 si se usa total_sectors_large */
    uint8_t media_type;
    uint16_t sectors_per_fat; /* 0 en FAT32 (usa sectors_per_fat_large) */
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_large;
    /* Extension FAT32 */
    uint32_t sectors_per_fat_large;
    uint16_t flags;
    uint16_t fat_version;
    uint32_t root_cluster;
    uint16_t fs_info_sector;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    uint8_t drive_number;
    uint8_t reserved1;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fs_type[8];
} __attribute__((packed)) boot_sector;

/* Entrada de directorio de 32 bytes (formato 8.3 / SHORT). */
typedef struct {
    uint8_t name[11];   /* 8.3, mayusculas, relleno con espacios */
    uint8_t attributes; /* bit 0x10 = directorio, 0x0F = LFN */
    uint8_t reserved;
    uint8_t creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t last_write_time;
    uint16_t last_write_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed)) fat32_dir_entry;

/* Atributos de una entrada de directorio. */
#define FAT32_ATTR_READ_ONLY 0x01
#define FAT32_ATTR_HIDDEN 0x02
#define FAT32_ATTR_SYSTEM 0x04
#define FAT32_ATTR_VOLUME_ID 0x08
#define FAT32_ATTR_DIRECTORY 0x10
#define FAT32_ATTR_ARCHIVE 0x20
#define FAT32_ATTR_LFN (0x0F)
#define FAT32_ATTR_LONG_NAME                                                                       \
    (FAT32_ATTR_READ_ONLY | FAT32_ATTR_HIDDEN | FAT32_ATTR_SYSTEM | FAT32_ATTR_VOLUME_ID)

/* Marcadores de la tabla FAT. */
#define FAT32_EOC 0x0FFFFFF8
#define FAT32_EOC_MASK 0x0FFFFFFF
#define FAT32_BAD_CLUSTER 0x0FFFFFF7
#define FAT32_FREE 0x00000000
#define FAT32_RESERVED 0x0FFFFFFF

/* Valor del primer cluster (root = 2 en FAT32). */
#define FAT32_FIRST_CLUSTER 2
