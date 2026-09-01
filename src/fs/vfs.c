/* vfs.c - Implementacion del Virtual File System.
   Gestiona la tabla de montajes y los descriptores de archivo abiertos,
   y redirige cada operacion (open/read/seek/close/readdir) al filesystem
   concreto montado en la ruta correspondiente. */

#include "fs/vfs.h"

/* Tablas globales del VFS. */
static vfs_mount_t mount_table[VFS_MAX_FILESYSTEMS];
static vfs_file_t open_files[VFS_MAX_OPEN_FILES];

/* Banderas de modo (simplificadas, sin permisos). */
#define VFS_MODE_READ 0x1
#define VFS_MODE_WRITE 0x2

/* whence para seek. */
#define VFS_SEEK_SET 0
#define VFS_SEEK_CUR 1
#define VFS_SEEK_END 2

static bool vfs_str_equal(const char *a, const char *b)
{
    while (*a && *b) {
        if (*a != *b) {
            return false;
        }
        a++;
        b++;
    }
    return *a == *b;
}

static int vfs_str_copy(char *dst, const char *src, uint32_t max)
{
    uint32_t i = 0;
    while (src[i] && i < max - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
    return 0;
}

/* Encuentra el punto de montaje mas profundo que contenga @path.
   La raiz se monta con mount_point "" o "/" y cuadra con cualquier ruta. */
static vfs_mount_t *vfs_find_mount(const char *path)
{
    vfs_mount_t *best = NULL;
    uint32_t best_len = 0;

    for (uint32_t i = 0; i < VFS_MAX_FILESYSTEMS; i++) {
        if (!mount_table[i].used) {
            continue;
        }
        const char *mp = mount_table[i].mount_point;
        /* Montaje raiz: "" o "/" cuadra con todas las rutas. */
        if (mp[0] == '\0' || (mp[0] == '/' && mp[1] == '\0')) {
            if (best == NULL) {
                best = &mount_table[i];
                best_len = 0;
            }
            continue;
        }
        /* Compara el prefijo de @path con el mount_point. */
        uint32_t j = 0;
        while (mp[j] && path[j] && mp[j] == path[j]) {
            j++;
        }
        /* Debe coincidir un componente completo de ruta: el siguiente
           caracter de path debe ser '/' o fin de cadena. */
        if (mp[j] == '\0' && (path[j] == '/' || path[j] == '\0') && j > best_len) {
            best = &mount_table[i];
            best_len = j;
        }
    }
    return best;
}

static int vfs_next_fd(void)
{
    for (int i = 0; i < VFS_MAX_OPEN_FILES; i++) {
        if (!open_files[i].used) {
            return i;
        }
    }
    return -1;
}

void vfs_init(void)
{
    for (uint32_t i = 0; i < VFS_MAX_FILESYSTEMS; i++) {
        mount_table[i].used = false;
        mount_table[i].ops = NULL;
        mount_table[i].fs = NULL;
        mount_table[i].mount_point[0] = '\0';
    }
    for (uint32_t i = 0; i < VFS_MAX_OPEN_FILES; i++) {
        open_files[i].used = false;
    }
}

int vfs_mount(const char *mount_point, vfs_ops_t *ops, void *fs)
{
    if (mount_point == NULL || ops == NULL || fs == NULL) {
        return -1;
    }

    for (uint32_t i = 0; i < VFS_MAX_FILESYSTEMS; i++) {
        if (mount_table[i].used) {
            continue;
        }
        mount_table[i].used = true;
        mount_table[i].ops = ops;
        mount_table[i].fs = fs;
        vfs_str_copy(mount_table[i].mount_point, mount_point, VFS_MOUNT_NAME_MAX);
        return 0;
    }
    return -2;
}

int vfs_umount(const char *mount_point)
{
    for (uint32_t i = 0; i < VFS_MAX_FILESYSTEMS; i++) {
        if (!mount_table[i].used) {
            continue;
        }
        if (vfs_str_equal(mount_table[i].mount_point, mount_point)) {
            mount_table[i].used = false;
            mount_table[i].ops = NULL;
            mount_table[i].fs = NULL;
            mount_table[i].mount_point[0] = '\0';
            return 0;
        }
    }
    return -1;
}

int vfs_open(const char *path, uint32_t mode)
{
    if (path == NULL) {
        return -1;
    }

    vfs_mount_t *mount = vfs_find_mount(path);
    if (mount == NULL || mount->ops == NULL || mount->ops->open == NULL) {
        return -2;
    }

    /* Construye el nodo raiz del filesystem montado. */
    vfs_node_t root;
    root.parent = NULL;
    root.name = path;
    root.inode = 0;
    root.size = 0;
    root.type = VFS_NODE_DIRECTORY;
    root.fs_data = mount->fs;

    void *handle = NULL;
    int res = mount->ops->open(mount->fs, &root, mode, &handle);
    if (res < 0) {
        return res;
    }

    int fd = vfs_next_fd();
    if (fd < 0) {
        if (mount->ops->close) {
            mount->ops->close(mount->fs, handle);
        }
        return -3;
    }

    open_files[fd].used = true;
    open_files[fd].mount = mount;
    open_files[fd].node = root;
    open_files[fd].handle = handle;
    open_files[fd].position = 0;

    return fd;
}

int vfs_read(int fd, void *buffer, uint32_t size)
{
    if (fd < 0 || fd >= VFS_MAX_OPEN_FILES || !open_files[fd].used) {
        return -1;
    }
    vfs_file_t *file = &open_files[fd];
    if (file->mount->ops->read == NULL) {
        return -2;
    }
    int bytes = file->mount->ops->read(file->mount->fs, file->handle, buffer, size);
    if (bytes > 0) {
        file->position += (uint32_t)bytes;
    }
    return bytes;
}

int vfs_seek(int fd, // NOLINT(bugprone-easily-swappable-parameters)
             int32_t offset, uint32_t whence)
{
    if (fd < 0 || fd >= VFS_MAX_OPEN_FILES || !open_files[fd].used) {
        return -1;
    }
    vfs_file_t *file = &open_files[fd];
    if (file->mount->ops->seek == NULL) {
        return -2;
    }
    int pos = file->mount->ops->seek(file->mount->fs, file->handle, offset, whence);
    if (pos >= 0) {
        file->position = (uint32_t)pos;
    }
    return pos;
}

int vfs_close(int fd)
{
    if (fd < 0 || fd >= VFS_MAX_OPEN_FILES || !open_files[fd].used) {
        return -1;
    }
    vfs_file_t *file = &open_files[fd];
    int res = 0;
    if (file->mount->ops->close) {
        res = file->mount->ops->close(file->mount->fs, file->handle);
    }
    file->used = false;
    file->handle = NULL;
    file->position = 0;
    return res;
}

int vfs_readdir(const char *path, char *out, uint32_t out_size)
{
    if (path == NULL || out == NULL) {
        return -1;
    }

    vfs_mount_t *mount = vfs_find_mount(path);
    if (mount == NULL || mount->ops == NULL || mount->ops->readdir == NULL) {
        return -2;
    }

    vfs_node_t dir;
    dir.parent = NULL;
    dir.name = path;
    dir.inode = 0;
    dir.size = 0;
    dir.type = VFS_NODE_DIRECTORY;
    dir.fs_data = mount->fs;

    uint32_t written = 0;
    for (uint32_t index = 0;; index++) {
        vfs_node_t entry;
        int res = mount->ops->readdir(mount->fs, &dir, index, &entry);
        if (res < 0) {
            return res;
        }
        if (res > 0) {
            break; /* fin del directorio */
        }

        /* Copia el nombre al buffer de salida. */
        const char *name = entry.name;
        uint32_t i = 0;
        while (name[i] && written + 1 < out_size) {
            out[written++] = name[i++];
        }
        if (written + 1 < out_size) {
            out[written++] = '\n';
        }
    }
    out[written] = '\0';
    return (int)written;
}
