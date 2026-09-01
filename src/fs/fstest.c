/* fstest.c - Pruebas de autocomprobacion del filesystem FAT32 + VFS.
   Monta el disco FAT32 a traves del driver ATA/IDE y valida operaciones
   de montaje, listado de directorios, apertura y lectura de archivos.
   Cada resultado se imprime por el puerto serie con un resumen final. */

#include "fs/fstest.h"
#include "drivers/ata.h"
#include "drivers/serial.h"
#include "fs/fat32.h"
#include "fs/vfs.h"
#include "util/util_lib.h"

static fat32_fs_t fs;

static uint32_t passed;
static uint32_t failed;

static void serial_str(const char *s)
{
    int64_t len = strlen(s);
    if (len > 0) {
        serial_write_string(COM1_BASE_ADDRESS, s, (uint32_t)len);
    }
}

/* Imprime un entero sin signo de 32 bits en decimal por el serie. */
static void serial_dec(uint32_t value)
{
    char buf[12];
    uint32_t p = 0;
    if (value == 0) {
        serial_write(COM1_BASE_ADDRESS, '0');
        return;
    }
    while (value > 0) {
        buf[p++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (p > 0) {
        serial_write(COM1_BASE_ADDRESS, (uint8_t)buf[--p]);
    }
}

/* Registra el resultado de una prueba individual. */
static void fs_test_report(const char *name, bool ok)
{
    if (ok) {
        passed++;
    } else {
        failed++;
    }
    serial_str("[FS ] ");
    serial_str(ok ? "OK  " : "FAIL");
    serial_str("  ");
    serial_str(name);
    serial_write(COM1_BASE_ADDRESS, '\n');
}

/* Verifica que @name aparezca dentro del listado @listing. */
static bool listing_contains(const char *listing, const char *name)
{
    uint32_t i = 0;
    while (listing[i] != '\0') {
        uint32_t j = 0;
        while (name[j] && listing[i + j] && name[j] == listing[i + j]) {
            j++;
        }
        if (name[j] == '\0') {
            return true;
        }
        while (listing[i] != '\0' && listing[i] != '\n') {
            i++;
        }
        if (listing[i] == '\n') {
            i++;
        }
    }
    return false;
}

/* Compara los primeros @prefix bytes de @buf con @expected. */
static bool starts_with(const uint8_t *buf, const char *expected)
{
    int64_t len = strlen(expected);
    for (int64_t i = 0; i < len; i++) {
        if ((uint8_t)buf[i] != (uint8_t)expected[i]) {
            return false;
        }
    }
    return true;
}

static void test_mount(void)
{
    int rc = fat32_mount(&fs, ata_read_sectors, NULL, "/");
    fs_test_report("montar FAT32 desde disco ATA", rc == 0);
}

static void test_readdir_root(void)
{
    char listing[512];
    int n = vfs_readdir("/", listing, sizeof(listing));
    bool ok = (n > 0) && listing_contains(listing, "HELLO.TXT");
    fs_test_report("listar raiz /", ok);
    if (ok) {
        serial_str("       /");
        serial_str(listing);
    }
}

static void test_readdir_docs(void)
{
    char listing[512];
    int n = vfs_readdir("/docs", listing, sizeof(listing));
    bool ok = (n > 0) && listing_contains(listing, "README.TXT");
    fs_test_report("listar /docs", ok);
    if (ok) {
        serial_str("       /docs");
        serial_str(listing);
    }
}

static void test_read_hello(void)
{
    int fd = vfs_open("/HELLO.TXT", 1);
    if (fd < 0) {
        fs_test_report("leer /HELLO.TXT", false);
        return;
    }
    uint8_t buf[256];
    int n = vfs_read(fd, buf, sizeof(buf));
    vfs_close(fd);
    bool ok = (n > 0) && starts_with(buf, "Hola desde el VFS FAT32.");
    fs_test_report("leer /HELLO.TXT", ok);
}

static void test_read_readme(void)
{
    int fd = vfs_open("/docs/README.TXT", 1);
    if (fd < 0) {
        fs_test_report("leer /docs/README.TXT", false);
        return;
    }
    uint8_t buf[512];
    int n = vfs_read(fd, buf, sizeof(buf));
    vfs_close(fd);
    bool ok = (n > 0) && starts_with(buf, "README del directorio docs.");
    fs_test_report("leer /docs/README.TXT", ok);
}

static void test_missing_file(void)
{
    int fd = vfs_open("/NOEXISTE.TXT", 1);
    fs_test_report("fallo al abrir archivo inexistente", fd < 0);
}

int fs_test_run(void)
{
    passed = 0;
    failed = 0;

    vfs_init();

    serial_str("\n[FS ] ===== Iniciando pruebas del filesystem FAT32 =====\n");
    test_mount();
    test_readdir_root();
    test_readdir_docs();
    test_read_hello();
    test_read_readme();
    test_missing_file();

    serial_str("[FS ] ===== Resumen: ");
    serial_dec(passed);
    serial_str(" OK, ");
    serial_dec(failed);
    serial_str(" fallidas =====\n");

    return failed == 0 ? 0 : -1;
}
