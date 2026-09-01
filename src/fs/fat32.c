/* fat32.c - Implementacion del driver de filesystem FAT32.
   Lee el bloque de arranque, sigue las cadenas de clusters a traves de
   la tabla FAT, recorre entradas de directorio y expone estas funciones
   como una implementacion de vfs_ops_t para el Virtual File System. */

#include "fs/fat32.h"
#include "fs/vfs.h"

/* Acceso little-endian a campos del buffer crudo. */

static uint8_t rd8(const uint8_t *p)
{
    return p[0];
}

static uint16_t rd16(const uint8_t *p)
{
    return (uint16_t)(p[0] | (p[1] << 8));
}

static uint32_t rd32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

typedef struct {
    fat32_fs_t *fs;
    uint32_t first_cluster;
    uint32_t size;
    uint32_t position;
    bool is_dir;
} fat32_file_t;

/* Lee un sector via el callback del dispositivo. */
static int fat32_read_sector(fat32_fs_t *fs, uint32_t sector, uint8_t *buffer)
{
    if (fs->read_sector == NULL) {
        return -1;
    }
    return fs->read_sector(sector, buffer, 1);
}

/* Convierte un numero de cluster a su primer sector de datos. */
static uint32_t fat32_cluster_to_sector(fat32_fs_t *fs, uint32_t cluster)
{
    return fs->first_data_sector + ((cluster - FAT32_FIRST_CLUSTER) * fs->bpb.sectors_per_cluster);
}

/* Lee la entrada de la tabla FAT para @cluster. Return: valor o negativo. */
static int32_t fat32_fat_entry(fat32_fs_t *fs, uint32_t cluster)
{
    if (cluster < FAT32_FIRST_CLUSTER || cluster >= fs->total_clusters + FAT32_FIRST_CLUSTER) {
        return -2;
    }
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = fs->first_fat_sector + (fat_offset / fs->bpb.bytes_per_sector);
    uint32_t in_sector = fat_offset % fs->bpb.bytes_per_sector;

    uint8_t sector[512];
    if (fat32_read_sector(fs, fat_sector, sector) < 0) {
        return -3;
    }
    /* La entrada puede quedar al final de un sector; lee el siguiente. */
    uint32_t value;
    if (in_sector + 4 > 512) {
        uint8_t next[512];
        if (fat32_read_sector(fs, fat_sector + 1, next) < 0) {
            return -3;
        }
        value = rd32(sector + in_sector) | (rd16(next) << 24);
    } else {
        value = rd32(sector + in_sector);
    }
    return (int32_t)(value & FAT32_EOC_MASK);
}

uint32_t fat32_next_cluster(fat32_fs_t *fs, uint32_t current)
{
    int32_t next = fat32_fat_entry(fs, current);
    if (next < 0 || (uint32_t)next >= FAT32_EOC) {
        return FAT32_EOC;
    }
    return (uint32_t)next;
}

/* Lee bytes de un archivo representado como cadena de clusters. */
// clang-format off
static uint32_t fat32_read_chain(fat32_fs_t *fs, uint32_t first_cluster, // NOLINT(bugprone-easily-swappable-parameters)
                                 uint32_t offset, uint8_t *buffer, uint32_t size)
// clang-format on
{
    if (first_cluster < FAT32_FIRST_CLUSTER) {
        return 0;
    }

    uint32_t cluster = first_cluster;
    uint32_t cluster_size = fs->cluster_size;
    uint32_t done = 0;
    uint32_t pos = offset;

    while (cluster < FAT32_EOC && done < size) {
        /* Avanza clusters vacios previos al offset. */
        uint32_t cluster_index = pos / cluster_size;

        while (cluster_index > 0 && cluster < FAT32_EOC) {
            cluster = fat32_next_cluster(fs, cluster);
            cluster_index--;
        }

        if (cluster >= FAT32_EOC) {
            break;
        }

        uint32_t within = pos % cluster_size;
        uint32_t to_read = cluster_size - within;

        if (to_read > (size - done)) {
            to_read = size - done;
        }

        uint32_t sector =
            fat32_cluster_to_sector(fs, cluster) + (within / fs->bpb.bytes_per_sector);

        uint32_t in_sector = within % fs->bpb.bytes_per_sector;

        uint32_t sector_count =
            (in_sector + to_read + fs->bpb.bytes_per_sector - 1) / fs->bpb.bytes_per_sector;

        uint8_t tmp[4096];

        if (sector_count * fs->bpb.bytes_per_sector > sizeof(tmp)) {
            sector_count = sizeof(tmp) / fs->bpb.bytes_per_sector;
            to_read = (sector_count * fs->bpb.bytes_per_sector) - in_sector;
        }

        if (fs->read_sector == NULL || fs->read_sector(sector, tmp, sector_count) < 0) {
            return done;
        }

        for (uint32_t i = 0; i < to_read; i++) {
            buffer[done++] = tmp[in_sector + i];
        }

        pos += to_read;
        cluster = fat32_next_cluster(fs, cluster);
    }
    return done;
}

int fat32_read_cluster(fat32_fs_t *fs, uint32_t cluster, uint8_t *buffer, uint32_t start_offset,
                       uint32_t count)
{
    return (int)fat32_read_chain(fs, cluster, start_offset, buffer, count);
}

/* Lee una entrada de directorio de cluster @dir_cluster en el indice
   @index (contando entradas de 32 bytes). Return: 1 = fin, 0 = ok,
   -1 = error. */
// clang-format off
static int fat32_read_dir_entry(fat32_fs_t *fs, uint32_t dir_cluster, // NOLINT(bugprone-easily-swappable-parameters)
                                uint32_t index, fat32_dir_entry *out)
// clang-format on
{
    uint32_t entry_offset = index * sizeof(fat32_dir_entry);

    /* Lee el bloque de 32 bytes desde la posicion exacta de la cadena. */
    uint8_t raw[32];
    uint32_t got = fat32_read_chain(fs, dir_cluster, entry_offset, raw, 32);

    if (got != 32) {
        return 1; /* fuera de la cadena: fin de directorio */
    }

    if (rd8(raw) == 0x00) {
        return 1; /* marcador de fin de directorio */
    }

    out->attributes = rd8(raw + 11);
    out->first_cluster_high = rd16(raw + 20);
    out->first_cluster_low = rd16(raw + 26);
    out->file_size = rd32(raw + 28);

    for (int i = 0; i < 11; i++) {
        out->name[i] = rd8(raw + i);
    }

    return 0;
}

/* Convierte un nombre 8.3 (11 bytes con espacios) a una cadena normal. */
static void fat32_short_name(const uint8_t *name11, char *out)
{
    int o = 0;
    int in_ext = false;
    for (int i = 0; i < 11; i++) {
        uint8_t c = name11[i];
        if (c == ' ') {
            continue;
        }
        if (i == 8) {
            out[o++] = '.';
            in_ext = true;
        }
        out[o++] = (char)c;
    }
    if (in_ext && o > 0 && out[o - 1] == '.') {
        o--; /* sin extension pero con punto: quitar el punto */
    }
    out[o] = '\0';
}

/* Determina el primer cluster de un archivo a partir de su entrada. */
static uint32_t fat32_entry_cluster(const fat32_dir_entry *e)
{
    return ((uint32_t)e->first_cluster_high << 16) | e->first_cluster_low;
}

/* Compara dos nombres ASCII sin distinguir mayusculas/minusculas,
   ya que FAT almacena los nombres 8.3 en mayusculas. */
static bool fat32_match(const char *name, const uint8_t *name11)
{
    char short_name[13];
    fat32_short_name(name11, short_name);
    uint32_t i = 0;
    while (name[i] && short_name[i]) {
        char a = name[i];
        char b = short_name[i];
        if (a >= 'a' && a <= 'z') {
            a = (char)(a - 'a' + 'A');
        }
        if (b >= 'a' && b <= 'z') {
            b = (char)(b - 'a' + 'A');
        }
        if (a != b) {
            return false;
        }
        i++;
    }
    return name[i] == '\0' && short_name[i] == '\0';
}

/* Busca la entrada llamada @comp dentro del directorio del cluster
   @dir_cluster. Si la encuentra, rellena @out.
   Return: 0 encontrado, 1 no encontrado, negativo error. */
static int fat32_find_in_dir(fat32_fs_t *fs, uint32_t dir_cluster, const char *comp,
                             fat32_dir_entry *out)
{
    for (uint32_t index = 0;; index++) {
        fat32_dir_entry entry;
        int r = fat32_read_dir_entry(fs, dir_cluster, index, &entry);
        if (r > 0) {
            return 1;
        }
        if (r < 0) {
            return r;
        }
        /* Las entradas LFN (atributo 0x0F) se ignoran al buscar. */
        if (entry.attributes == FAT32_ATTR_LFN) {
            continue;
        }
        /* "." y ".." se ignoran aqui. */
        if (entry.name[0] == 0x2E) {
            continue;
        }
        if (fat32_match(comp, entry.name)) {
            *out = entry;
            return 0;
        }
    }
}

/* Resuelve una ruta partiendo de root_cluster.
   @path: ruta completa tipo "/dir/archivo"
   @out_cluster: cluster del objeto encontrado
   @out_size: tamaño
   @out_is_dir: true si es directorio
   Return: 0 ok, negativo error, 1 no encontrado. */
// clang-format off
static int fat32_resolve(fat32_fs_t *fs, const char *path,
                         uint32_t *out_cluster, // NOLINT(bugprone-easily-swappable-parameters)
                         uint32_t *out_size, bool *out_is_dir)
// clang-format on
{
    uint32_t current = fs->root_cluster;
    uint32_t size = 0;
    bool is_dir = true;

    const char *p = path;
    /* Omitir separadores iniciales. */
    while (*p == '/') {
        p++;
    }

    while (*p != '\0') {
        /* Extrae el siguiente componente (hasta '/' o fin). */
        char comp[VFS_NAME_MAX];
        uint32_t ci = 0;
        while (*p != '\0' && *p != '/' && ci < VFS_NAME_MAX - 1) {
            comp[ci++] = *p++;
        }
        comp[ci] = '\0';

        /* Busca el componente en el directorio actual (current). */
        fat32_dir_entry match;
        int r = fat32_find_in_dir(fs, current, comp, &match);
        if (r != 0) {
            return r < 0 ? r : 1;
        }

        current = fat32_entry_cluster(&match);
        size = match.file_size;
        is_dir = (match.attributes & FAT32_ATTR_DIRECTORY) != 0;

        while (*p == '/') {
            p++;
        }
        if (is_dir && *p == '\0') {
            /* Nos detenemos en un directorio. */
            break;
        }
    }

    *out_cluster = current;
    *out_size = size;
    *out_is_dir = is_dir;
    return 0;
}

/* ---- Implementacion de vfs_ops_t ---- */

static int fat32_ops_open(void *fs_ptr, vfs_node_t *node, uint32_t mode, void **handle)
{
    (void)mode;
    fat32_fs_t *fs = (fat32_fs_t *)fs_ptr;
    if (fs == NULL || node == NULL || node->name == NULL) {
        return -1;
    }

    uint32_t cluster;
    uint32_t size;
    bool is_dir;
    int r = fat32_resolve(fs, node->name, &cluster, &size, &is_dir);
    if (r != 0) {
        return r < 0 ? r : -4;
    }

    /* Usa memoria estatica para no depender de malloc. */
    static fat32_file_t pool[16];
    for (int i = 0; i < 16; i++) {
        fat32_file_t *f = &pool[i];
        /* Reutilizacion simple: sobreescribimos la primera entrada libre
           rastreado por binosy de primer cluster == 0. */
        if (f->fs == NULL) {
            f->fs = fs;
            f->first_cluster = cluster;
            f->size = size;
            f->position = 0;
            f->is_dir = is_dir;
            *handle = f;
            return 0;
        }
    }
    return -5;
}

static int fat32_ops_read(void *fs_ptr, // NOLINT(bugprone-easily-swappable-parameters)
                          void *handle, void *buffer, uint32_t size)
{
    (void)fs_ptr;
    fat32_file_t *f = (fat32_file_t *)handle;
    if (f == NULL || f->fs == NULL) {
        return -1;
    }
    if (f->is_dir) {
        return 0;
    }
    if (f->position >= f->size) {
        return 0;
    }
    uint32_t remaining = f->size - f->position;
    if (size > remaining) {
        size = remaining;
    }
    uint32_t got = fat32_read_chain(f->fs, f->first_cluster, f->position, (uint8_t *)buffer, size);
    f->position += got;
    return (int)got;
}

static int fat32_ops_seek(void *fs_ptr, // NOLINT(bugprone-easily-swappable-parameters)
                          void *handle,
                          int32_t offset, // NOLINT(bugprone-easily-swappable-parameters)
                          uint32_t whence)
{
    (void)fs_ptr;
    fat32_file_t *f = (fat32_file_t *)handle;
    if (f == NULL) {
        return -1;
    }

    uint32_t base = 0;
    if (whence == 1) { /* SEEK_CUR */
        base = f->position;
    } else if (whence == 2) { /* SEEK_END */
        base = f->size;
    }

    if (offset < 0 && (uint32_t)(-offset) > base) {
        f->position = 0;
    } else {
        f->position = base + (uint32_t)offset;
    }
    return (int)f->position;
}

static int fat32_ops_close(void *fs_ptr, // NOLINT(bugprone-easily-swappable-parameters)
                           void *handle)
{
    (void)fs_ptr;
    fat32_file_t *f = (fat32_file_t *)handle;
    if (f == NULL) {
        return -1;
    }
    f->fs = NULL;
    return 0;
}

static int fat32_ops_readdir(void *fs_ptr, vfs_node_t *dir_node, uint32_t index, vfs_node_t *out)
{
    fat32_fs_t *fs = (fat32_fs_t *)fs_ptr;
    if (fs == NULL || dir_node == NULL || out == NULL || dir_node->name == NULL) {
        return -1;
    }

    uint32_t cluster;
    uint32_t size;
    bool is_dir;
    int r = fat32_resolve(fs, dir_node->name, &cluster, &size, &is_dir);
    if (r != 0) {
        return r < 0 ? r : 1;
    }
    if (!is_dir) {
        return -2;
    }

    for (;;) {
        fat32_dir_entry entry;
        int rr = fat32_read_dir_entry(fs, cluster, index, &entry);
        if (rr > 0) {
            return 1; /* fin */
        }
        if (rr < 0) {
            return rr;
        }
        index++;
        if (rd8((uint8_t *)entry.name) == 0xE5 || /* borrada */
            entry.attributes & FAT32_ATTR_VOLUME_ID) {
            continue;
        }
        if (entry.attributes == FAT32_ATTR_LFN) {
            continue;
        }

        out->parent = NULL;
        out->inode = fat32_entry_cluster(&entry);
        out->size = entry.file_size;
        out->type = (entry.attributes & FAT32_ATTR_DIRECTORY) ? VFS_NODE_DIRECTORY : VFS_NODE_FILE;
        out->fs_data = fs;
        /* Nombre estatico por entrada (basta para esta demo). */
        static char names[16][VFS_NAME_MAX];
        static uint32_t counter = 0;
        uint32_t slot = (counter++) & 15;
        fat32_short_name(entry.name, names[slot]);
        out->name = names[slot];
        return 0;
    }
}

/* ---- Montaje ---- */

static const vfs_ops_t fat32_vfs_ops = {
    fat32_ops_open, fat32_ops_read, fat32_ops_seek, fat32_ops_close, fat32_ops_readdir,
};

int fat32_mount(fat32_fs_t *fs, fat32_read_sector_fn read, fat32_write_sector_fn write,
                const char *mount_point)
{
    if (fs == NULL || read == NULL) {
        return -1;
    }

    fs->read_sector = read;
    fs->write_sector = write;

    /* Lee el sector de arranque. */
    uint8_t sector[512];
    if (read(0, sector, 1) < 0) {
        return -2;
    }

    /* Rellena la BPB desde el buffer crudo (independiente del alineado). */
    fs->bpb.bytes_per_sector = rd16(sector + 11);
    fs->bpb.sectors_per_cluster = rd8(sector + 13);
    fs->bpb.reserved_sectors = rd16(sector + 14);
    fs->bpb.num_fats = rd8(sector + 16);
    fs->bpb.total_sectors = rd16(sector + 19);
    fs->bpb.total_sectors_large = rd32(sector + 32);
    fs->bpb.sectors_per_fat_large = rd32(sector + 36);
    fs->bpb.root_cluster = rd32(sector + 44);

    if (fs->bpb.sectors_per_fat_large == 0 || fs->bpb.bytes_per_sector == 0 ||
        fs->bpb.sectors_per_cluster == 0) {
        return -3;
    }

    if (fs->bpb.bytes_per_sector > 4096) {
        fs->bpb.bytes_per_sector = 4096;
    }

    fs->first_fat_sector = fs->bpb.reserved_sectors;
    fs->first_data_sector =
        fs->bpb.reserved_sectors + (fs->bpb.num_fats * fs->bpb.sectors_per_fat_large);

    uint32_t total_sectors =
        (fs->bpb.total_sectors != 0) ? fs->bpb.total_sectors : fs->bpb.total_sectors_large;
    if (total_sectors == 0 || total_sectors <= fs->first_data_sector) {
        return -3;
    }
    fs->total_data_sectors = total_sectors - fs->first_data_sector;
    fs->total_clusters = fs->total_data_sectors / fs->bpb.sectors_per_cluster;
    fs->cluster_size = fs->bpb.sectors_per_cluster * fs->bpb.bytes_per_sector;
    fs->root_cluster = fs->bpb.root_cluster;

    fs->ops = fat32_vfs_ops;

    if (mount_point != NULL) {
        return vfs_mount(mount_point, &fs->ops, fs);
    }
    return 0;
}
