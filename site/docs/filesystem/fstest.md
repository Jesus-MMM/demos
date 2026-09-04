---
title: "Pruebas del filesystem"
order: 3
---

# Pruebas del filesystem (`include/fs/fstest.h` / `src/fs/fstest.c`)

## Qué es

`fs_test_run()` es una rutina de **autocomprobación** que se ejecuta durante el arranque del kernel (desde `kernel_main`). Monta el disco FAT32 vía el driver ATA con el VFS y valida:

1. El montaje del disco.
2. El listado de directorios.
3. La lectura de archivos de prueba.
4. El manejo de errores (archivo inexistente).

## Flujo de la prueba

```mermaid
flowchart TB
    A[fs_test_run] --> B[vfs_init]
    B --> C[fat32_mount via ata_read_sectors → /]
    C --> D{¿Montaje OK?}
    D -->|No| Z[Imprimir error]
    D -->|Sí| E[Leer directorio raíz /]
    E --> F[Buscar docs/]
    F --> G[Leer directorio /docs]
    G --> H[Leer /HELLO.TXT]
    H --> I[Leer /docs/README.TXT]
    I --> J[Intentar leer archivo inexistente]
    J --> K{¿Error correcto?}
    K -->|Sí| L[Imprimir todas las pruebas OK]
    K -->|No| M[Imprimir fallo]
    Z --> END[Fin]
    L --> END
    M --> END
```

## Comportamiento al arrancar

Al ejecutarse, `fs_test_run()` imprime por el puerto serie el resultado, como:

```
[FS] VFS initialized
[FS] Mounting FAT32 on '/'...
[FS] Mount OK
[FS] Listing root:
HELLO.TXT
FONDO.PNG
docs
[FS] Listing /docs:
README.TXT
[FS] Reading /HELLO.TXT: Hello from DemOS!
[FS] Reading /docs/README.TXT: ...
[FS] Testing error: opening /NOTEXIST.TXT -> expected error OK
[FS] All filesystem tests passed
```

Si algo falla, se imprime el punto exacto del error para facilitar la depuración.

## Relación con el resto del sistema

```mermaid
graph LR
    MAIN[kernel_main] --> FSTEST[fs_test_run]
    FSTEST --> VFS[vfs_init / vfs_mount]
    VFS --> FAT32[fat32_mount]
    FAT32 --> ATA[ata_read_sectors]
    ATA --> DISK[demos.img]
```

> **Nota**: `fs_test_run()` se ejecuta **antes** del cambio al modo gráfico, por lo que puede usar el puerto serie sin interferir con la pantalla.
