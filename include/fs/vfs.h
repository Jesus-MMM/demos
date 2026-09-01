/* vfs.h - Virtual File System del kernel.
   Proporciona una capa de abstraccion sobre los filesystems: un sistema
   de montajes, nodos abstractos con operaciones (open/read/close/readdir)
   y resolucion de rutas tipo "/dir/file". Los filesystems concretos
   (p.ej. FAT32) registran una implementacion de vfs_ops_t para servir
   estas operaciones. */

#pragma once

#include "types.h"

#define VFS_MAX_PATH 256
#define VFS_MAX_FILESYSTEMS 8
#define VFS_MAX_OPEN_FILES 32
#define VFS_NAME_MAX 64
#define VFS_MOUNT_NAME_MAX 32

typedef struct vfs_node vfs_node_t;

/* Tipos de objeto dentro de un filesystem. */
enum vfs_node_type {
    VFS_NODE_UNKNOWN = 0,
    VFS_NODE_FILE = 1,
    VFS_NODE_DIRECTORY = 2,
};

/* Operaciones que debe implementar cada filesystem (vfs_ops_t).
   Todas reciben un puntero a la implementacion concreta (@fs) que se
   asocia al punto de montaje y devuelven 0 en exito o un codigo de error. */
typedef struct vfs_ops {
    /* Abre el nodo indicado para lectura/escritura segun @mode.
       Devuelve un handler (>= 0) o un valor negativo en error. */
    int (*open)(void *fs, vfs_node_t *node, uint32_t mode, void **handle);
    /* Lee hasta @size bytes desde la posicion actual del handler. */
    int (*read)(void *fs, void *handle, void *buffer, uint32_t size);
    /* Avanza o retrocede la posicion de lectura del handler. */
    int (*seek)(void *fs, void *handle, int32_t offset, uint32_t whence);
    /* Cierra y libera el handler. */
    int (*close)(void *fs, void *handle);
    /* Lee una entrada de directorio en @index y la guarda en @out.
       Devuelve 0 con datos, 1 al llegar al final, o negativo en error. */
    int (*readdir)(void *fs, vfs_node_t *node, uint32_t index, vfs_node_t *out);
} vfs_ops_t;

/* Nodo abstracto: identifica un objeto dentro de un filesystem. */
struct vfs_node {
    vfs_node_t *parent;
    const char *name;
    uint32_t inode; /* identificador interno del filesystem */
    uint32_t size;  /* tamaño en bytes (0 si directorio) */
    enum vfs_node_type type;
    void *fs_data; /* datos privados del filesystem */
};

/* Punto de montaje: une un filesystem concreto a una ruta del VFS. */
typedef struct vfs_mount {
    bool used;
    char mount_point[VFS_MOUNT_NAME_MAX];
    vfs_ops_t *ops;
    void *fs; /* instancia concreta del filesystem */
} vfs_mount_t;

/* Archivo abierto: combina nodo + mount + handle del filesystem. */
typedef struct vfs_file {
    bool used;
    vfs_mount_t *mount;
    vfs_node_t node;
    void *handle; /* handler devuelto por ops->open */
    uint32_t position;
} vfs_file_t;

/* Inicializa el VFS (tabla de montajes y de archivos abiertos). */
void vfs_init(void);

/* Monta un filesystem en una ruta. Devuelve 0 en exito. */
int vfs_mount(const char *mount_point, vfs_ops_t *ops, void *fs);

/* Desmonta el filesystem montado en @mount_point. Devuelve 0 en exito. */
int vfs_umount(const char *mount_point);

/* Abre la ruta @path y devuelve un descriptor de archivo (>= 0). */
int vfs_open(const char *path, uint32_t mode);

/* Lee hasta @size bytes del descriptor @fd. Devuelve bytes leidos. */
int vfs_read(int fd, void *buffer, uint32_t size);

/* Mueve la posicion de lectura del descriptor @fd. */
int vfs_seek(int fd, int32_t offset, uint32_t whence);

/* Cierra el descriptor @fd. Devuelve 0 en exito. */
int vfs_close(int fd);

/* Lista el contenido del directorio @path (solo nombres, uno por linea)
   imprimiendo cada entrada en el buffer @out. */
int vfs_readdir(const char *path, char *out, uint32_t out_size);
