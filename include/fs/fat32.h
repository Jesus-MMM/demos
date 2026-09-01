/* fat32.h - Driver de filesystem FAT32 sobre el VFS.
   Proporciona una interfaz de dispositivo de bloques (leer/escribir
   sectores) e implementa las operaciones vfs_ops_t para permitir
   montar una particion FAT32 y leer archivos/directorios a traves
   del Virtual File System. */

#pragma once

#include "fs/filesystem.h"
#include "fs/vfs.h"
#include "types.h"

/* Llamada de lectura/escritura de un sector del dispositivo de bloques.
   @sector: numero de sector (LBA)
   @buffer: destino/origen de los datos
   @count:  numero de sectores consecutivos
   Return:  0 en exito, negativo en error */
typedef int (*fat32_read_sector_fn)(uint32_t sector, uint8_t *buffer, uint32_t count);
typedef int (*fat32_write_sector_fn)(uint32_t sector, const uint8_t *buffer, uint32_t count);

/* Instancia de un filesystem FAT32 montado. */
typedef struct {
    vfs_ops_t ops;              /* operaciones expuestas al VFS */
    boot_sector bpb;            /* parametros del bloque de arranque */
    uint32_t first_fat_sector;  /* sector inicial de la primera FAT */
    uint32_t first_data_sector; /* sector inicial de la region de datos */
    uint32_t root_cluster;      /* cluster de la raiz */
    uint32_t total_data_sectors;
    uint32_t total_clusters;
    uint32_t cluster_size; /* bytes por cluster */

    fat32_read_sector_fn read_sector;
    fat32_write_sector_fn write_sector;
} fat32_fs_t;

/* Inicializa la instancia @fs a partir del dispositivo de bloques.
   Si @mount_point no es NULL, monta el filesystem en esa ruta del VFS.
   Return: 0 en exito, negativo en error. */
int fat32_mount(fat32_fs_t *fs, fat32_read_sector_fn read, fat32_write_sector_fn write,
                const char *mount_point);

/* Lee @cluster_size bytes del cluster @cluster hacia @buffer.
   @start_offset: desplazamiento dentro del cluster (0 = inicio).
   @count: bytes a leer (como maximo cluster_size - start_offset).
   Return: bytes leidos o negativo en error. */
int fat32_read_cluster(fat32_fs_t *fs, uint32_t cluster, uint8_t *buffer, uint32_t start_offset,
                       uint32_t count);

/* Sigue la cadena de clusters desde @first hasta @target (0 = siguiente).
   Return: numero de cluster o FAT32_EOC si se alcanza el final. */
uint32_t fat32_next_cluster(fat32_fs_t *fs, uint32_t current);

/* Guarda en @out el nodo correspondiente a la ruta @path dentro del
   filesystem (funcion de ayuda para implementar ops->open). */
int fat32_lookup(fat32_fs_t *fs, const char *path, vfs_node_t *out);
