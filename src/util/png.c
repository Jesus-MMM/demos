/* png.c - Herramienta del kernel para decodificar imagenes PNG,
   Implementa DEFLATE (RFC 1951) y zlib (RFC 1950) desde cero, los
   filtros de PNG y la conversion de cada pixel a un indice de la paleta VGA
   activa. */

#include "util/png.h"
#define MAXBITS 15

/* ----------------------------- lector de bits ----------------------------- */

typedef struct {
    const uint8_t *data;
    uint32_t size;
    uint32_t pos;
    uint32_t bitbuf; /* proximo bit en el bit 0 (LSB-first, como deflate) */
    int bitcnt;      /* bits validos en bitbuf */
} BR;

static void br_fill(BR *b)
{
    while (b->bitcnt <= 24 && b->pos < b->size) {
        b->bitbuf |= (uint32_t)b->data[b->pos++] << b->bitcnt;
        b->bitcnt += 8;
    }
}

static int br_bit(BR *b)
{
    if (b->bitcnt == 0) {
        br_fill(b);
    }
    if (b->bitcnt == 0) {
        return -1;
    }
    int v = (int)(b->bitbuf & 1);
    b->bitbuf >>= 1;
    b->bitcnt--;
    return v;
}

static int br_bits(BR *b, int n)
{
    int v = 0;
    for (int i = 0; i < n; i++) {
        int bit = br_bit(b);
        if (bit < 0) {
            return -1;
        }
        v |= bit << i;
    }
    return v;
}

static void br_align(BR *b)
{
    b->bitbuf >>= (b->bitcnt & 7);
    b->bitcnt &= ~7;
}

static int br_byte(BR *b)
{
    if (b->bitcnt >= 8) {
        int c = (int)(b->bitbuf & 0xff);
        b->bitbuf >>= 8;
        b->bitcnt -= 8;
        return c;
    }
    if (b->pos < b->size) {
        return b->data[b->pos++];
    }
    return -1;
}

/* ------------------------------ arbol huffman ----------------------------- */

typedef struct {
    int maxbits;
    int cnt[MAXBITS + 1];   /* cantidad de codigos de cada longitud */
    int index[MAXBITS + 1]; /* indice del primer simbolo de cada longitud */
    uint16_t order[288];    /* simbolos ordenados por (longitud, valor) */
} HUFF;

static void set_mem(void *dst, int value, uint32_t len)
{
    uint8_t *d = (uint8_t *)dst;
    for (uint32_t i = 0; i < len; i++) {
        d[i] = (uint8_t)value;
    }
}

static int huff_build(HUFF *h, const uint8_t *lens, int n)
{
    set_mem(h, 0, sizeof(*h));
    int maxbits = 0;
    for (int i = 0; i < n; i++) {
        int l = lens[i];
        if (l > MAXBITS) {
            return -1;
        }
        h->cnt[l]++;
        if (l > maxbits) {
            maxbits = l;
        }
    }
    if (maxbits == 0) {
        return -1;
    }
    h->maxbits = maxbits;

    int acc = 0;
    for (int l = 1; l <= maxbits; l++) {
        h->index[l] = acc;
        acc += h->cnt[l];
    }

    int total = 0;
    for (int l = 1; l <= maxbits; l++) {
        total += h->cnt[l] << (maxbits - l);
    }
    if (total > (1 << maxbits)) {
        return -1;
    }

    int k = 0;
    for (int l = 1; l <= maxbits; l++) {
        for (int s = 0; s < n; s++) {
            if (lens[s] == l) {
                h->order[k++] = (uint16_t)s;
            }
        }
    }
    return 0;
}

static int huff_decode(BR *b, HUFF *h)
{
    int code = 0;
    int first = 0;
    int index = 0;
    for (int l = 1; l <= h->maxbits; l++) {
        int bit = br_bit(b);
        if (bit < 0) {
            return -1;
        }
        code = (code << 1) | bit;
        if (code - first < h->cnt[l]) {
            return h->order[index + (code - first)];
        }
        index += h->cnt[l];
        first = (first + h->cnt[l]) << 1;
    }
    return -1;
}

/* -------------------------------- inflate --------------------------------- */

typedef struct {
    uint8_t *data;
    uint32_t size, cap;
} OUT;

static int out_putc(OUT *o, uint8_t c)
{
    if (o->size >= o->cap) {
        return -1;
    }
    o->data[o->size++] = c;
    return 0;
}

static const int bbase[29] = {3,  4,  5,  6,  7,  8,  9,  10, 11,  13,  15,  17,  19,  23, 27,
                              31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
static const int bextr[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2,
                              2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
static const int dbase[30] = {1,    2,    3,    4,    5,    7,    9,    13,    17,    25,
                              33,   49,   65,   97,   129,  193,  257,  385,   513,   769,
                              1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
static const int dextr[30] = {0, 0, 0, 0, 1, 1, 2, 2,  3,  3,  4,  4,  5,  5,  6,
                              6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
static const int cl_order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

static void fixed_trees(HUFF *lit, HUFF *dist)
{
    uint8_t ll[288];
    uint8_t dl[30];
    for (int i = 0; i < 288; i++) {
        ll[i] = (uint8_t)((i < 144) ? 8 : (i < 256) ? 9 : (i < 280) ? 7 : 8);
    }
    for (int i = 0; i < 30; i++) {
        dl[i] = 5;
    }
    huff_build(lit, ll, 288);
    huff_build(dist, dl, 30);
}

static int inflate(const uint8_t *in, uint32_t inlen, OUT *out, uint32_t *consumed)
{
    if (inlen < 6) {
        return -1;
    }
    if ((in[0] & 0x0f) != 8) {
        return -1;
    }
    if (((uint32_t)in[0] * 256 + in[1]) % 31 != 0) {
        return -1;
    }

    BR b = {in + 2, inlen - 2, 0, 0, 0};
    HUFF lit, dist, clen;

    for (;;) {
        int bfinal = br_bit(&b);
        int bt0 = br_bit(&b), bt1 = br_bit(&b);
        if (bfinal < 0 || bt0 < 0 || bt1 < 0) {
            return -1;
        }
        int btype = (bt1 << 1) | bt0;

        if (btype == 0) {
            br_align(&b);
            int l0 = br_byte(&b), l1 = br_byte(&b);
            int n0 = br_byte(&b), n1 = br_byte(&b);
            if (l0 < 0 || l1 < 0 || n0 < 0 || n1 < 0) {
                return -1;
            }
            if ((l0 + l1 * 256) + (n0 + n1 * 256) != 0xffff) {
                return -1;
            }
            int len = l0 + l1 * 256;
            if (out->size + (uint32_t)len > out->cap) {
                return -1;
            }
            for (int i = 0; i < len; i++) {
                int c = br_byte(&b);
                if (c < 0) {
                    return -1;
                }
                out->data[out->size++] = (uint8_t)c;
            }
        } else if (btype == 1) {
            fixed_trees(&lit, &dist);
            goto decode;
        } else if (btype == 2) {
            int hlit = br_bits(&b, 5) + 257;
            int hdist = br_bits(&b, 5) + 1;
            int hclen = br_bits(&b, 4) + 4;
            if (hlit > 286 || hdist > 30 || hclen > 19) {
                return -1;
            }
            uint8_t clens[19] = {0};
            for (int i = 0; i < hclen; i++) {
                clens[cl_order[i]] = (uint8_t)br_bits(&b, 3);
            }
            if (huff_build(&clen, clens, 19)) {
                return -1;
            }

            uint8_t lens[288 + 32];
            set_mem(lens, 0, sizeof(lens));
            int total = hlit + hdist, idx = 0;
            while (idx < total) {
                int s = huff_decode(&b, &clen);
                if (s < 0) {
                    return -1;
                }
                if (s < 16) {
                    lens[idx++] = (uint8_t)s;
                } else if (s == 16) {
                    if (idx == 0) {
                        return -1;
                    }
                    int rep = 3 + br_bits(&b, 2);
                    uint8_t prev = lens[idx - 1];
                    while (rep-- && idx < total) {
                        lens[idx++] = prev;
                    }
                } else if (s == 17) {
                    int rep = 3 + br_bits(&b, 3);
                    idx += rep;
                } else {
                    int rep = 11 + br_bits(&b, 7);
                    idx += rep;
                }
            }
            if (huff_build(&lit, lens, hlit)) {
                return -1;
            }
            if (huff_build(&dist, lens + hlit, hdist)) {
                return -1;
            }
            goto decode;
        } else {
            return -1;
        }

        if (bfinal) {
            break;
        }
        continue;

    decode:
        for (;;) {
            int s = huff_decode(&b, &lit);
            if (s < 0) {
                return -1;
            }
            if (s < 256) {
                if (out_putc(out, (uint8_t)s)) {
                    return -1;
                }
            } else if (s == 256) {
                break;
            } else {
                if (s > 285) {
                    return -1;
                }
                int li = s - 257;
                int len = bbase[li] + br_bits(&b, bextr[li]);
                int ds = huff_decode(&b, &dist);
                if (ds < 0 || ds > 29) {
                    return -1;
                }
                int dist = dbase[ds] + br_bits(&b, dextr[ds]);
                if ((uint32_t)dist > out->size || out->size + (uint32_t)len > out->cap) {
                    return -1;
                }
                uint8_t *src = out->data + out->size - dist;
                for (int i = 0; i < len; i++) {
                    out->data[out->size++] = src[i];
                }
            }
        }
        if (bfinal) {
            break;
        }
    }
    if (consumed) {
        *consumed = 2 + (b.pos - b.bitcnt / 8);
    }
    return 0;
}

/* ------------------------------ filtros png ------------------------------ */

static int ia(int v)
{
    return v < 0 ? -v : v;
}

static int paeth(int a, int b, int c)
{
    int p = a + b - c;
    int pa = ia(p - a);
    int pb = ia(p - b);
    int pc = ia(p - c);
    if (pa <= pb && pa <= pc) {
        return a;
    }
    if (pb <= pc) {
        return b;
    }
    return c;
}

/* ----------------------- mapeo RGB a paleta VGA ----------------------- */

static uint32_t rd_be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}

static int mem_eq(const void *a, const void *b, uint32_t n)
{
    const uint8_t *x = (const uint8_t *)a;
    const uint8_t *y = (const uint8_t *)b;
    for (uint32_t i = 0; i < n; i++) {
        if (x[i] != y[i]) {
            return 0;
        }
    }
    return 1;
}

/* Tabla de consulta 4-4-4 -> mejor indice de la paleta VGA activa. */
static void build_lut(const uint8_t (*dac)[3], uint8_t lut[4096])
{
    for (uint32_t idx = 0; idx < 4096; idx++) {
        int r4 = (int)((idx >> 8) & 0xF);
        int g4 = (int)((idx >> 4) & 0xF);
        int b4 = (int)(idx & 0xF);
        int tr = r4 * 255 / 15;
        int tg = g4 * 255 / 15;
        int tb = b4 * 255 / 15;
        uint32_t best = 0;
        uint32_t bestd = 0xFFFFFFFFu;
        for (uint32_t e = 0; e < 256; e++) {
            int dr = tr - ((int)dac[e][0] * 255 / 63);
            int dg = tg - ((int)dac[e][1] * 255 / 63);
            int db = tb - ((int)dac[e][2] * 255 / 63);
            uint32_t d = (uint32_t)((dr * dr) + (dg * dg) + (db * db));
            if (d < bestd) {
                bestd = d;
                best = e;
            }
        }
        lut[idx] = (uint8_t)best;
    }
}

static uint8_t lut_pixel(const uint8_t *lut, uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t idx = ((uint32_t)(r >> 4) << 8) | ((uint32_t)(g >> 4) << 4) | (b >> 4);
    return lut[idx];
}

/* ----------------------------- lectura de png ----------------------------- */

static const uint8_t SIG[8] = {137, 80, 78, 71, 13, 10, 26, 10};
static uint8_t raw_scratch[PNG_RAW_MAX]; /* output inflado: rows + byte de filtro */

bool png_decode_indexed(const uint8_t *file, uint32_t file_size, const uint8_t (*dac)[3],
                       uint8_t *out_pixels, uint32_t out_capacity, uint32_t *out_w, uint32_t *out_h)
{
    if (out_h == NULL || file == NULL || dac == NULL || out_pixels == NULL || out_w == NULL) {
        return -1;
    }
    if (file_size < 8 || !mem_eq(file, SIG, 8)) {
        return -2;
    }

    uint32_t w = 0;
    uint32_t h = 0;
    int bitdepth = 0;
    int coltype = 0;
    int got_ihdr = 0;
    const uint8_t *pal = NULL;
    uint32_t paln = 0;
    uint32_t idatn = 0;
    uint8_t idat[PNG_FILE_MAX];

    uint32_t ip = 8;
    while (ip + 12 <= file_size) {
        uint32_t clen = rd_be32(file + ip);
        const uint8_t *type = file + ip + 4;
        const uint8_t *cdat = file + ip + 8;
        if (clen > file_size - ip - 12) {
            return -3;
        }
        if (mem_eq(type, "IHDR", 4)) {
            if (clen < 13) {
                return -3;
            }
            w = rd_be32(cdat);
            h = rd_be32(cdat + 4);
            bitdepth = cdat[8];
            coltype = cdat[9];
            int comp = cdat[10];
            int filter = cdat[11];
            int interlace = cdat[12];
            if (comp != 0 || filter != 0) {
                return -4;
            }
            if (interlace != 0) {
                return -5;
            }
            got_ihdr = 1;
        } else if (mem_eq(type, "PLTE", 4) && clen >= 1) {
            pal = cdat;
            paln = clen / 3;
        } else if (mem_eq(type, "IDAT", 4)) {
            if (idatn + clen > (uint32_t)sizeof(idat)) {
                return -6;
            }
            for (uint32_t i = 0; i < clen; i++) {
                idat[idatn++] = cdat[i];
            }
        } else if (mem_eq(type, "IEND", 4)) {
            break;
        }
        ip += 12 + clen;
    }

    if (!got_ihdr || idatn == 0) {
        return -7;
    }

    static const int chans[7] = {1, 0, 3, 1, 2, 0, 4};
    int ok = 0;
    if (coltype == 0) {
        ok = (bitdepth == 1 || bitdepth == 2 || bitdepth == 4 || bitdepth == 8 || bitdepth == 16);
    } else if (coltype == 2 || coltype == 4 || coltype == 6) {
        ok = (bitdepth == 8 || bitdepth == 16);
    } else if (coltype == 3) {
        ok = (bitdepth == 1 || bitdepth == 2 || bitdepth == 4 || bitdepth == 8);
    }
    if (!ok) {
        return -8;
    }

    OUT raw = {raw_scratch, 0, (uint32_t)sizeof(raw_scratch)};
    uint32_t consumed = 0;
    if (inflate(idat, idatn, &raw, &consumed)) {
        return -9;
    }

    int channels = chans[coltype];
    uint32_t bitspp = (uint32_t)channels * (uint32_t)bitdepth;
    uint32_t rowbytes = ((w * bitspp) + 7) / 8;
    uint32_t stride = rowbytes + 1;
    if (raw.size < stride * h) {
        return -10;
    }
    if (w * h > out_capacity) {
        return -11;
    }

    int bpp = channels * bitdepth / 8;
    if (bpp < 1) {
        bpp = 1;
    }

    /* aplicar reversa de filtros */
    for (uint32_t y = 0; y < h; y++) {
        uint8_t *row = raw.data + (y * stride);
        uint8_t *cur = row + 1;
        uint8_t *prev = (y == 0) ? NULL : raw.data + ((y - 1) * stride) + 1;
        uint8_t filt = row[0];
        for (uint32_t x = 0; x < rowbytes; x++) {
            uint8_t a = (x >= (uint32_t)bpp) ? cur[x - bpp] : 0;
            uint8_t bb = prev ? prev[x] : 0;
            uint8_t cc = (prev && x >= (uint32_t)bpp) ? prev[x - bpp] : 0;
            if (filt == 1) {
                cur[x] = (uint8_t)(cur[x] + a);
            } else if (filt == 2) {
                cur[x] = (uint8_t)(cur[x] + bb);
            } else if (filt == 3) {
                cur[x] = (uint8_t)(cur[x] + ((a + bb) >> 1));
            } else if (filt == 4) {
                cur[x] = (uint8_t)(cur[x] + (uint8_t)paeth(a, bb, cc));
            } else if (filt != 0) {
                return -12;
            }
        }
    }

    uint8_t lut[4096];
    build_lut(dac, lut);

    /* mapear cada pixel a un indice de la paleta VGA activa */
    for (uint32_t y = 0; y < h; y++) {
        const uint8_t *row = raw.data + (y * stride) + 1;
        for (uint32_t x = 0; x < w; x++) {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            if (bitdepth == 8) {
                if (coltype == 0) {
                    r = g = b = row[x];
                } else if (coltype == 3) {
                    uint8_t pi = row[x];
                    if (pi >= paln) {
                        return -13;
                    }
                    r = pal[3 * pi];
                    g = pal[(3 * pi) + 1];
                    b = pal[(3 * pi) + 2];
                } else if (coltype == 2) {
                    r = row[3 * x];
                    g = row[(3 * x) + 1];
                    b = row[(3 * x) + 2];
                } else if (coltype == 4) {
                    r = g = b = row[2 * x];
                } else {
                    r = row[4 * x];
                    g = row[(4 * x) + 1];
                    b = row[(4 * x) + 2];
                }
            } else if (bitdepth == 16) {
                uint32_t k = x * channels * 2;
                if (coltype == 0) {
                    r = g = b = row[k];
                } else if (coltype == 2) {
                    r = row[k];
                    g = row[k + 2];
                    b = row[k + 4];
                } else if (coltype == 4) {
                    r = g = b = row[k];
                } else {
                    r = row[k];
                    g = row[k + 2];
                    b = row[k + 4];
                }
            } else {
                /* bitdepth 1/2/4: escala de grises (0) o paleta (3) */
                uint32_t bx = x * bitspp;
                uint32_t bytep = bx >> 3;
                uint32_t bitoff = 8 - bitdepth - (bx & 7);
                uint32_t mask = (1U << bitdepth) - 1;
                uint32_t v = (row[bytep] >> bitoff) & mask;
                if (coltype == 0) {
                    uint32_t scaled = v * 255 / mask;
                    r = g = b = (uint8_t)scaled;
                } else {
                    if (v >= paln) {
                        return -13;
                    }
                    r = pal[3 * v];
                    g = pal[(3 * v) + 1];
                    b = pal[(3 * v) + 2];
                }
            }
            out_pixels[(y * w) + x] = lut_pixel(lut, r, g, b);
        }
    }

    *out_w = w;
    *out_h = h;
    return 0;
}
