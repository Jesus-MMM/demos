---
title: "Splash"
order: 3
---

# Splash — pantalla de presentación (`include/util/splash.h` / `src/util/splash.c`)

## Qué es

`splash.c` implementa una **pantalla de presentación animada** del sistema operativo (`animate_splash()`), que se muestra después de la inicialización básica del kernel.

## Estado actual

> **Nota**: la animación está **comentada** en `main.c` (`// animate_splash();`), porque el arranque actual salta directamente al modo gráfico y al escritorio. Los archivos `splash.c`, `big_text.c` y `timer.c` siguen existiendo y documentados, listos para reactivarse si se desea la animación de arranque.

## `animate_splash()`

```c
void animate_splash(void);
```

Muestra una animación de inicio con **texto grande (5x5)** y **recuadros** usando las funciones de `big_text.c`, avanzando por tiempo mediante `timer.c`.

## Flujo de la animación

```mermaid
flowchart TD
    A[animate_splash] --> B[Limpiar pantalla en modo texto]
    B --> C[Dibujar recuadro decorativo<br/>draw_box]
    C --> D[Dibujar caracteres grandes 'DemOS'<br/>draw_big_char con glifos 5x5]
    D --> E[Animación: alternar colores /<br/>parpadeo por tiempo (timer)]
    E --> F[Esperar / pausa]
    F --> G[Terminar animación]
```

## Dependencias

| Módulo | Uso |
|--------|-----|
| `drivers/timer.h` | Control de tiempo de la animación |
| `drivers/vga_legacy.h` | Salida en modo texto |
| `util/big_text.h` | Caracteres grandes y recuadros |
