#ifndef CANVAS_H_
#define CANVAS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef CANVASDEF
#define CANVASDEF static inline
#endif

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

#define CANVAS_PIXEL(c, x, y) (c).pixels[(y) * (c).width + (x)]

CANVASDEF Canvas create_canvas(size_t width, size_t height, uint32_t *pixels);
CANVASDEF void free_canvas(Canvas *c);
CANVASDEF void clear_background(Canvas *c, uint32_t color);
CANVASDEF void canvas_rect(Canvas *c, Rectangle rec, uint32_t color);

static uint32_t crc32_table[256];
CANVASDEF void make_crc32_table(void);
CANVASDEF uint32_t crc32(const uint8_t *buf, size_t len);
CANVASDEF uint32_t adler32(const uint8_t *data, size_t len);
CANVASDEF int write_be32(FILE *f, uint32_t v);
CANVASDEF int write_chunk(FILE *f, const char type[4], const uint8_t *data, uint32_t len);
CANVASDEF int write_png_from_rgba32(const char *filename, const uint32_t *pixels, uint32_t width, uint32_t height);

#ifdef CANVAS_IMPLEMENTATION

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
            CANVAS_PIXEL(*c, x, y) = color;
        }
    }
}

CANVASDEF void canvas_rect(Canvas *c, Rectangle rec, uint32_t color) {
    for (size_t y = rec.y; y < rec.y + rec.h && y < c->height; ++y) {
        for (size_t x = rec.x; x < rec.x + rec.w && x < c->width; ++x) {
            CANVAS_PIXEL(*c, x, y) = color;
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

#endif // CANVAS_IMPLEMENTATION

#endif // CANVAS_H_