/* png.h - Herramienta del kernel para decodificar imagenes PNG.
   inflate/zlib propios, filtros de PNG y
   conversion de cada pixel (RGB/RGBA/escala de grises/paleta) a un indice de
   la paleta VGA activa. */

#pragma once

#include "types.h"

#define PNG_FILE_MAX 40000    /* tamano maximo del archivo PNG en disco */
#define PNG_RAW_MAX 256200    /* output inflado maximo (320x200 RGBA + filtros) */
#define PNG_PIXELS_MAX 64000  /* 320 * 200 */

/** png_decode_indexed - Decodifica un PNG y emite un indice de paleta por
   pixel, mapeando cada canal a la paleta VGA @dac (256 entradas de 6 bits:
   [r][g][b] en 0..63). No usa entrelazado Adam7 ni bitdepth 16 para color
   3 (paleta).
   @file:        contenido completo del archivo PNG
   @file_size:   tamano de @file
   @dac:         paleta VGA activa
   @out_pixels:  buffer de salida (w*h indices de 8 bits)
   @out_capacity: capacidad de @out_pixels en bytes
   @out_w,@out_h: dimensiones decodificadas
   Return: true en exito, false en error. */
bool png_decode_indexed(const uint8_t *file, uint32_t file_size, const uint8_t (*dac)[3],
                       uint8_t *out_pixels, uint32_t out_capacity, uint32_t *out_w,
                       uint32_t *out_h);
