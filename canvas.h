#ifndef CANVAS_H_
#define CANVAS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef CANVASDEF
/*
   Define CANVASDEF before including this file to control function linkage.
   Example: #define CANVASDEF static inline
   This is useful for single-file projects to let the compiler inline functions
   and remove unused ones.
*/
#define CANVASDEF
#endif /* CANVASDEF */

typedef struct {
    size_t x, y, w, h;
} Rectangle;

typedef struct {
    size_t width, height;
    // 0xRRGGBBAA
    uint32_t *pixels;
} Canvas;

CANVASDEF Canvas create_canvas(size_t width, size_t height, uint32_t *pixels);
CANVASDEF void free_canvas(Canvas *c);
CANVASDEF void clear_background(Canvas *c, uint32_t color);

CANVASDEF int32_t RGB(uint8_t r, uint8_t g, uint8_t b);
CANVASDEF int32_t RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

CANVASDEF void canvas_putpixel(Canvas *c, int x, int y, uint32_t color);
CANVASDEF uint32_t canvas_getpixel(const Canvas *c, int x, int y, uint32_t fallback);

CANVASDEF void canvas_hline(Canvas *c, int x0, int x1, int y, uint32_t color);
CANVASDEF void canvas_vline(Canvas *c, int x, int y0, int y1, uint32_t color);
CANVASDEF void canvas_line(Canvas *c, int x0, int y0, int x1, int y1, uint32_t color);

CANVASDEF void canvas_rect(Canvas *c, Rectangle rec, uint32_t color);
CANVASDEF void canvas_rect_fill(Canvas *c, Rectangle rec, uint32_t color);

CANVASDEF void canvas_circle(Canvas *c, int cx, int cy, int r, uint32_t color);
CANVASDEF void canvas_circle_fill(Canvas *c, int cx, int cy, int r, uint32_t color);

CANVASDEF void canvas_triangle(Canvas *c, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
CANVASDEF void canvas_triangle_fill(Canvas *c, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);

/* PNG encoder */
static uint32_t crc32_table[256];
CANVASDEF void make_crc32_table(void);
CANVASDEF uint32_t crc32(const uint8_t *buf, size_t len);
CANVASDEF uint32_t adler32(const uint8_t *data, size_t len);
CANVASDEF int write_be32(FILE *f, uint32_t v);
CANVASDEF int write_chunk(FILE *f, const char type[4], const uint8_t *data, uint32_t len);
CANVASDEF int write_png_from_rgba32(const char *filename, const uint32_t *pixels, uint32_t width, uint32_t height);

typedef struct {
    FILE *f;
    size_t width, height;
    uint8_t *y_plane;
    uint8_t *u_plane;
    uint8_t *v_plane;
} Y4MWriter;


CANVASDEF Y4MWriter *y4m_start(const char *filename, size_t width, size_t height, int fps);
CANVASDEF void y4m_write_frame(Y4MWriter *w, const Canvas *c);
CANVASDEF void y4m_end(Y4MWriter *w);

#ifdef CANVAS_IMPLEMENTATION

/* ---------- helpers (internal) ---------- */
CANVASDEF int canvas__imax(int a, int b) {
    return a > b ? a : b;
}
CANVASDEF int canvas__imin(int a, int b) {
    return a < b ? a : b;
}
CANVASDEF void canvas__swap_int(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

CANVASDEF Canvas create_canvas(size_t width, size_t height, uint32_t *pixels) {
    return (Canvas) {
        .width = width,
        .height = height,
        .pixels = pixels
    };
}

CANVASDEF void free_canvas(Canvas *c) {
    c->pixels = NULL;
    c->width = c->height = 0;
}

CANVASDEF void clear_background(Canvas *c, uint32_t color) {
    for (size_t y = 0; y < c->height; ++y) {
        for (size_t x = 0; x < c->width; ++x) {
            canvas_putpixel(c, (int)x, (int)y, color);
        }
    }
}

CANVASDEF int32_t RGB(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | 255;
}

CANVASDEF int32_t RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | a;
}

CANVASDEF void canvas_putpixel(Canvas *c, int x, int y, uint32_t color) {
    if (!c || !c->pixels) return;
    if (x < 0 || y < 0 || x >= (int)c->width || y >= (int)c->height) return;
    c->pixels[(size_t)y * c->width + (size_t)x] = color;
}

CANVASDEF uint32_t canvas_getpixel(const Canvas *c, int x, int y, uint32_t fallback) {
    if (!c || !c->pixels) return fallback;
    if (x < 0 || y < 0 || x >= (int)c->width || y >= (int)c->height) return fallback;
    return c->pixels[(size_t)y * c->width + (size_t)x];
}

CANVASDEF void canvas_hline(Canvas *c, int x0, int x1, int y, uint32_t color) {
    if (!c || !c->pixels) return;
    if (y < 0 || y >= (int)c->height) return;
    if (x0 > x1) canvas__swap_int(&x0, &x1);
    if (x1 < 0 || x0 >= (int)c->width) return;
    if (x0 < 0) x0 = 0;
    if (x1 >= (int)c->width) x1 = (int)c->width - 1;

    uint32_t *row = c->pixels + (size_t)y * c->width + (size_t)x0;
    for (int x = x0; x <= x1; ++x) *row++ = color;
}

CANVASDEF void canvas_vline(Canvas *c, int x, int y0, int y1, uint32_t color) {
    if (!c || !c->pixels) return;
    if (x < 0 || x >= (int)c->width) return;
    if (y0 > y1) canvas__swap_int(&y0, &y1);
    if (y1 < 0 || y0 >= (int)c->height) return;
    if (y0 < 0) y0 = 0;
    if (y1 >= (int)c->height) y1 = (int)c->height - 1;

    uint32_t *p = c->pixels + (size_t)y0 * c->width + (size_t)x;
    for (int y = y0; y <= y1; ++y) {
        *p = color;
        p += c->width;
    }
}

CANVASDEF void canvas_line(Canvas *c, int x0, int y0, int x1, int y1, uint32_t color) {
    if (!c || !c->pixels) return;
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (1) {
        canvas_putpixel(c, (int)x0, (int)y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

CANVASDEF void canvas_rect(Canvas *c, Rectangle rec, uint32_t color) {
    if (rec.w <= 0 || rec.h <= 0) return;
    size_t x0 = rec.x, y0 = rec.y, x1 = rec.x + rec.w - 1, y1 = rec.y + rec.h - 1;
    canvas_hline(c, (int)x0, (int)x1, (int)y0, color);
    canvas_hline(c, (int)x0, (int)x1, (int)y1, color);
    canvas_vline(c, (int)x0, (int)y0, (int)y1, color);
    canvas_vline(c, (int)x1, (int)y0, (int)y1, color);

}

CANVASDEF void canvas_rect_fill(Canvas *c, Rectangle rec, uint32_t color) {
    if (!c || !c->pixels || rec.w == 0 || rec.h == 0) return;
    size_t y_end = rec.y + rec.h;
    size_t x_end = rec.x + rec.w;
    if (rec.y >= c->height || rec.x >= c->width) return;
    if (y_end > c->height) y_end = c->height;
    if (x_end > c->width)  x_end = c->width;

    for (size_t y = rec.y; y < y_end; ++y) {
        uint32_t *row = c->pixels + y * c->width + rec.x;
        for (size_t x = rec.x; x < x_end; ++x) *row++ = color;
    }
}

CANVASDEF void canvas_circle(Canvas *c, int cx, int cy, int r, uint32_t color) {
    if (!c || !c->pixels || r <= 0) return;
    int x = r, y = 0;
    int err = 1 - x;
    while (x >= y) {
        canvas_putpixel(c, (int)cx + x, (int)cy + y, color);
        canvas_putpixel(c, (int)cx + y, (int)cy + x, color);
        canvas_putpixel(c, (int)cx + y, (int)cy + x, color);
        canvas_putpixel(c, (int)cx - y, (int)cy + x, color);
        canvas_putpixel(c, (int)cx - x, (int)cy + y, color);
        canvas_putpixel(c, (int)cx - x, (int)cy - y, color);
        canvas_putpixel(c, (int)cx - y, (int)cy - x, color);
        canvas_putpixel(c, (int)cx + y, (int)cy - x, color);
        canvas_putpixel(c, (int)cx + x, (int)cy - y, color);
        ++y;
        if (err < 0) err += 2 * y + 1;
        else {
            --x;
            err += 2 * (y - x) + 1;
        }
    }
}

CANVASDEF void canvas_circle_fill(Canvas *c, int cx, int cy, int r, uint32_t color) {
    if (!c || !c->pixels || r <= 0) return;
    int x = r, y = 0;
    int err = 1 - x;
    while (x >= y) {
        /* draw spans across symmetrical rows */
        canvas_hline(c, cx - x, cx + x, cy + y, color);
        canvas_hline(c, cx - x, cx + x, cy - y, color);
        canvas_hline(c, cx - y, cx + y, cy + x, color);
        canvas_hline(c, cx - y, cx + y, cy - x, color);
        ++y;
        if (err < 0) err += 2 * y + 1;
        else {
            --x;
            err += 2 * (y - x) + 1;
        }
    }
}

CANVASDEF void canvas_triangle(Canvas *c, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    canvas_line(c, x0, y0, x1, y1, color);
    canvas_line(c, x1, y1, x2, y2, color);
    canvas_line(c, x2, y2, x0, y0, color);
}

/* Helpers for filled triangle */
CANVASDEF void canvas__fill_flat_bottom(Canvas *c, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    if (y1 == y0) return;
    float invslope1 = (float)(x1 - x0) / (float)(y1 - y0);
    float invslope2 = (float)(x2 - x0) / (float)(y2 - y0);
    float curx1 = (float)x0;
    float curx2 = (float)x0;
    for (int y = y0; y <= y1; ++y) {
        int xa = (int)(curx1 + 0.5f);
        int xb = (int)(curx2 + 0.5f);
        if (xa > xb) canvas__swap_int(&xa, &xb);
        canvas_hline(c, xa, xb, y, color);
        curx1 += invslope1;
        curx2 += invslope2;
    }
}
CANVASDEF void canvas__fill_flat_top(Canvas *c, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    if (y2 == y0) return;
    float invslope1 = (float)(x2 - x0) / (float)(y2 - y0);
    float invslope2 = (float)(x2 - x1) / (float)(y2 - y1);
    float curx1 = (float)x2;
    float curx2 = (float)x2;
    for (int y = y2; y >= y0; --y) {
        int xa = (int)(curx1 + 0.5f);
        int xb = (int)(curx2 + 0.5f);
        if (xa > xb) canvas__swap_int(&xa, &xb);
        canvas_hline(c, xa, xb, y, color);
        curx1 -= invslope1;
        curx2 -= invslope2;
    }
}

CANVASDEF void canvas_triangle_fill(Canvas *c, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    /* sort by y (y0 <= y1 <= y2) */
    if (y1 < y0) {
        canvas__swap_int(&y0, &y1);
        canvas__swap_int(&x0, &x1);
    }
    if (y2 < y0) {
        canvas__swap_int(&y0, &y2);
        canvas__swap_int(&x0, &x2);
    }
    if (y2 < y1) {
        canvas__swap_int(&y1, &y2);
        canvas__swap_int(&x1, &x2);
    }

    if (y0 == y2) { /* degenerate: all on one scanline */
        int xa = canvas__imin(canvas__imin(x0, x1), x2);
        int xb = canvas__imax(canvas__imax(x0, x1), x2);
        canvas_hline(c, xa, xb, y0, color);
        return;
    }

    if (y1 == y0) {
        /* flat-top */
        if (x1 < x0) canvas__swap_int(&x0, &x1);
        canvas__fill_flat_top(c, x0, y0, x1, y1, x2, y2, color);
    } else if (y1 == y2) {
        /* flat-bottom */
        if (x2 < x1) canvas__swap_int(&x1, &x2);
        canvas__fill_flat_bottom(c, x0, y0, x1, y1, x2, y2, color);
    } else {
        /* general: split at y1 on edge 0-2 */
        int x3 = x0 + (int)((float)(y1 - y0) / (float)(y2 - y0) * (float)(x2 - x0) + 0.5f);
        /* two flat triangles */
        if (x1 < x3) {
            canvas__fill_flat_bottom(c, x0, y0, x1, y1, x3, y1, color);
            canvas__fill_flat_top(c, x1, y1, x3, y1, x2, y2, color);
        } else {
            canvas__fill_flat_bottom(c, x0, y0, x3, y1, x1, y1, color);
            canvas__fill_flat_top(c, x3, y1, x1, y1, x2, y2, color);
        }
    }
}

CANVASDEF void make_crc32_table(void) {
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t c = i;
        for (int k = 0; k < 8; ++k) {
            if (c & 1) c = 0xEDB88320U ^ (c >> 1);
            else       c = c >> 1;
        }
        crc32_table[i] = c;
    }
}

CANVASDEF uint32_t crc32(const uint8_t *buf, size_t len) {
    uint32_t c = 0xFFFFFFFFU;
    for (size_t i = 0; i < len; ++i)
        c = crc32_table[(c ^ buf[i]) & 0xFF] ^ (c >> 8);
    return c ^ 0xFFFFFFFFU;
}

CANVASDEF uint32_t adler32(const uint8_t *data, size_t len) {
    const uint32_t MOD_ADLER = 65521U;
    uint32_t a = 1, b = 0;
    for (size_t i = 0; i < len; ++i) {
        a = (a + data[i]) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }
    return (b << 16) | a;
}

CANVASDEF int write_be32(FILE *f, uint32_t v) {
    uint8_t b[4];
    b[0] = (v >> 24) & 0xFF;
    b[1] = (v >> 16) & 0xFF;
    b[2] = (v >> 8) & 0xFF;
    b[3] = v & 0xFF;
    return (fwrite(b, 1, 4, f) == 4) ? 0 : -1;
}

CANVASDEF int write_chunk(FILE *f, const char type[4], const uint8_t *data, uint32_t len) {
    if (write_be32(f, len) != 0) return -1;
    if (fwrite(type, 1, 4, f) != 4) return -1;
    if (len > 0 && fwrite(data, 1, len, f) != len) return -1;

    // compute CRC over type + data
    uint8_t *crc_buf = (uint8_t*)malloc(4 + len);
    if (!crc_buf) return -1;
    memcpy(crc_buf, type, 4);
    if (len) memcpy(crc_buf + 4, data, len);
    uint32_t crc = crc32(crc_buf, 4 + len);
    free(crc_buf);

    if (write_be32(f, crc) != 0) return -1;
    return 0;
}

CANVASDEF int write_png_from_rgba32(const char *filename, const uint32_t *pixels, uint32_t width, uint32_t height) {
    if (!filename || !pixels || width == 0 || height == 0) return -1;
#if defined(_MSC_VER)
    FILE *f = NULL;
    if (fopen_s(&f, filename, "wb") != 0 || !f) {
        perror("Cannot open file");
        return -1;
    }
#else
    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("Cannot open file");
        return -1;
    }
#endif


    make_crc32_table();

    // PNG signature
    const uint8_t png_sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    if (fwrite(png_sig, 1, 8, f) != 8) goto fail;

    // ---- IHDR chunk data (13 bytes) ----
    uint8_t ihdr[13];
    ihdr[0] = (width >> 24) & 0xFF;
    ihdr[1] = (width >> 16) & 0xFF;
    ihdr[2] = (width >> 8) & 0xFF;
    ihdr[3] = width & 0xFF;
    ihdr[4] = (height >> 24) & 0xFF;
    ihdr[5] = (height >> 16) & 0xFF;
    ihdr[6] = (height >> 8) & 0xFF;
    ihdr[7] = height & 0xFF;
    ihdr[8] = 8;    // bit depth
    ihdr[9] = 6;    // color type = 6 (RGBA)
    ihdr[10] = 0;   // compression
    ihdr[11] = 0;   // filter
    ihdr[12] = 0;   // interlace

    if (write_chunk(f, "IHDR", ihdr, 13) != 0) goto fail;

    // ---- Create raw scanline data: each row: [filter=0][R][G][B][A] * width ----
    // raw_size = height * (1 + width*4)
    size_t row_bytes = 1 + (size_t)width * 4;
    size_t raw_size = (size_t)height * row_bytes;

    uint8_t *raw = (uint8_t*)malloc(raw_size);
    if (!raw) {
        perror("malloc raw");
        goto fail;
    }
    for (uint32_t y = 0; y < height; ++y) {
        uint8_t *row = raw + (size_t)y * row_bytes;
        row[0] = 0x00; // filter type 0 (None)
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t p = pixels[(size_t)y * width + x];
            // p assumed 0xRRGGBBAA
            row[1 + x * 4 + 0] = (p >> 24) & 0xFF; // R
            row[1 + x * 4 + 1] = (p >> 16) & 0xFF; // G
            row[1 + x * 4 + 2] = (p >> 8) & 0xFF; // B
            row[1 + x * 4 + 3] = p & 0xFF;       // A
        }
    }

    // ---- Build zlib stream (header + stored blocks + adler32) in a buffer ----
    // zlib header: 0x78 0x01 (CM=8, CINFO=7, FCHECK/FDICT/FLEVEL appropriate for no preset)
    // stored block format: [1 byte header][2 bytes LEN (LE)][2 bytes NLEN (LE)][data...]
    // header byte: low 3 bits are BFINAL (bit0) and BTYPE (bits1-2). For stored and final -> 0x01
    // For non-final stored blocks -> 0x00

    // number of stored blocks:
    const size_t MAX_STORED = 65535;
    size_t nblocks = (raw_size + MAX_STORED - 1) / MAX_STORED;

    // idat_size = 2 (zlib header) + raw_size + nblocks*5 (each stored block has 1 header byte + 2 LEN + 2 NLEN)
    // plus 4 bytes Adler32
    size_t idat_size = 2 + raw_size + nblocks * 5 + 4;
    uint8_t *idat = (uint8_t*)malloc(idat_size);
    if (!idat) {
        perror("malloc idat");
        free(raw);
        goto fail;
    }

    size_t pos = 0;
    // zlib header
    idat[pos++] = 0x78;
    idat[pos++] = 0x01;

    // stored blocks
    size_t remaining = raw_size;
    size_t offset = 0;
    for (size_t bi = 0; bi < nblocks; ++bi) {
        uint16_t this_len = (uint16_t)(remaining > MAX_STORED ? MAX_STORED : remaining);
        uint16_t nlen = (uint16_t)(~this_len);

        uint8_t header_byte = (bi == nblocks - 1) ? 0x01 : 0x00; // final? -> 1 : 0 ; BTYPE=00 so 0x01 or 0x00
        idat[pos++] = header_byte;

        // write LEN and NLEN (little-endian)
        idat[pos++] = (uint8_t)(this_len & 0xFF);
        idat[pos++] = (uint8_t)((this_len >> 8) & 0xFF);
        idat[pos++] = (uint8_t)(nlen & 0xFF);
        idat[pos++] = (uint8_t)((nlen >> 8) & 0xFF);

        // copy block data
        memcpy(idat + pos, raw + offset, this_len);
        pos += this_len;
        offset += this_len;
        remaining -= this_len;
    }

    // adler32 (big-endian)
    uint32_t adl = adler32(raw, raw_size);
    idat[pos++] = (adl >> 24) & 0xFF;
    idat[pos++] = (adl >> 16) & 0xFF;
    idat[pos++] = (adl >> 8) & 0xFF;
    idat[pos++] = adl & 0xFF;

    if (pos != idat_size) {
        fprintf(stderr, "inconsistent idat size: pos=%lu idat_size=%lu\n", (unsigned long)pos, (unsigned long)idat_size);
        free(idat);
        free(raw);
        goto fail;
    }

    // ---- Write IDAT chunk ----
    if (write_chunk(f, "IDAT", idat, (uint32_t)idat_size) != 0) {
        free(idat);
        free(raw);
        goto fail;
    }

    free(idat);
    free(raw);

    // ---- IEND chunk (zero-length) ----
    if (write_chunk(f, "IEND", NULL, 0) != 0) goto fail;

    fclose(f);
    return 0;

fail:
    if (f) fclose(f);
    return -1;
}

CANVASDEF Y4MWriter *y4m_start(const char *filename, size_t width, size_t height, int fps) {
    Y4MWriter *w = malloc(sizeof(*w));
    if (!w) return NULL;

#if defined(_MSC_VER)
    if (fopen_s(&w->f, filename, "wb") != 0) {
        free(w);
        return NULL;
    }
#else
    w->f = fopen(filename, "wb");
    if (!w->f) {
        free(w);
        return NULL;
    }
#endif

    w->width = width;
    w->height = height;

    // Allocate planes once
    w->y_plane = malloc(width * height);
    w->u_plane = malloc(width * height);
    w->v_plane = malloc(width * height);
    if (!w->y_plane || !w->u_plane || !w->v_plane) {
        fclose(w->f);
        free(w->y_plane);
        free(w->u_plane);
        free(w->v_plane);
        free(w);
        return NULL;
    }

    // Write YUV4MPEG2 header
    fprintf(w->f, "YUV4MPEG2 W%lu H%lu F%d:1 Ip A1:1 C444\n", (unsigned long)width, (unsigned long)height, fps);
    return w;
}

CANVASDEF void y4m_write_frame(Y4MWriter *w, const Canvas *c) {
    fprintf(w->f, "FRAME\n");

    for (size_t i = 0; i < w->width * w->height; i++) {
        uint8_t r = (c->pixels[i] >> 24) & 0xFF;
        uint8_t g = (c->pixels[i] >> 16) & 0xFF;
        uint8_t b = (c->pixels[i] >> 8) & 0xFF;

        w->y_plane[i] = (uint8_t)( 0.299 * r + 0.587 * g + 0.114 * b);
        w->u_plane[i] = (uint8_t)(-0.169 * r - 0.331 * g + 0.5 * b + 128);
        w->v_plane[i] = (uint8_t)( 0.5 * r - 0.419 * g - 0.081 * b + 128);
    }

    fwrite(w->y_plane, 1, w->width * w->height, w->f);
    fwrite(w->u_plane, 1, w->width * w->height, w->f);
    fwrite(w->v_plane, 1, w->width * w->height, w->f);
}

CANVASDEF void y4m_end(Y4MWriter *w) {
    if (!w) return;
    fclose(w->f);
    free(w->y_plane);
    free(w->u_plane);
    free(w->v_plane);
    free(w);
}

#endif // CANVAS_IMPLEMENTATION

#endif // CANVAS_H_