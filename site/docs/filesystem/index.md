---
title: "Filesystem"
order: 4
---

# Filesystem

El sistema de archivos de DemOS permite leer archivos de un disco virtual FAT32. Se compone de dos capas:

1. **VFS (Virtual File System)** — una capa de abstracción que unifica el acceso a distintos filesystems.
2. **FAT32** — el driver concreto que implementa las operaciones del VFS sobre un disco ATA/IDE.

## Arquitectura

```mermaid
graph TB
    subgraph "Aplicación"
        MAIN[background_load_from_disk<br/>leer /FONDO.PNG]
        FSTEST[fs_test_run<br/>pruebas de autocomprobación]
    end

    subgraph "VFS"
        VFS[VFS<br/>vfs_init / mount / open / read]
        MOUNT[Tabla de montajes<br/>VFS_MAX_FILESYSTEMS=8]
        OFILE[Tabla de archivos abiertos<br/>VFS_MAX_OPEN_FILES=32]
    end

    subgraph "Driver FAT32"
        FAT32[fat32_fs_t<br/>BPB + geometría]
        OPS[vfs_ops_t<br/>open/read/seek/close/readdir]
    end

    subgraph "Hardware"
        ATA[ata_read_sectors<br/>PIO LBA28]
        DISK[Disco demos.img<br/>FAT32 32MB]
    end

    MAIN --> VFS
    FSTEST --> VFS
    VFS -->|vfs_mount| FAT32
    VFS --> OPS
    FAT32 --> OPS
    OPS --> FAT32
    FAT32 --> ATA
    ATA --> DISK
```

## En esta sección

- [VFS](./vfs/) — capa de abstracción del sistema de archivos
- [FAT32](./fat32/) — driver de filesystem FAT32
- [Pruebas del filesystem](./fstest/) — autocomprobación al arrancar
