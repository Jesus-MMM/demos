/* fstest.h - Pruebas de autocomprobacion del filesystem FAT32 + VFS.
   Monta el disco FAT32 (leido via el driver ATA/IDE) y ejecuta una serie
   de verificaciones de montaje, listado de directorios y lectura de
   archivos, imprimiendo el resultado por el puerto serie. */

#pragma once

/** fs_test_run - Ejecuta las pruebas del filesystem sobre el disco ATA.
    Inicializa el VFS, monta la particion FAT32 y valida su contenido.
    Return: 0 si todas las pruebas pasan, otro valor en caso contrario. */
int fs_test_run(void);
