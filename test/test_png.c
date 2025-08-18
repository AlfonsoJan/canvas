#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CANVASDEF static inline
#define CANVAS_IMPLEMENTATION
#include "../canvas.h"

#include "test.h"

static unsigned char* read_all(const char* path, size_t* out_len){
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n <= 0) { fclose(f); return NULL; }
    unsigned char* buf = (unsigned char*)malloc((size_t)n);
    if (!buf){ fclose(f); return NULL; }
    size_t rd = fread(buf,1,(size_t)n,f);
    fclose(f);
    if (rd != (size_t)n){ free(buf); return NULL; }
    *out_len = (size_t)n;
    return buf;
}

static uint32_t be32(const unsigned char* p){
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|((uint32_t)p[3]);
}

int main() {
    uint32_t px[W*H] = {
        0xFF0000FF, 0x00FF00FF, 0x0000FFFF,
        0x00000000, 0xFFFFFFFF, 0x11223344
    };
    const char* path = "build/tests_out_tiny.png";
    int rc = write_png_from_rgba32(path, px, W, H);
    ASSERT_EQ_I(rc, 0);

    // Parse the PNG
    size_t n=0; unsigned char* buf = read_all(path, &n);
    ASSERT_TRUE(buf && n>0);

    // Signature
    const unsigned char sig[8] = {137,80,78,71,13,10,26,10};
    ASSERT_TRUE(n>8 && memcmp(buf, sig, 8)==0);
    size_t p = 8;

    make_crc32_table();

    // IHDR
    ASSERT_TRUE(p+8 <= n);
    uint32_t ihdr_len = be32(buf+p); p+=4;
    ASSERT_EQ_I(ihdr_len, 13);
    ASSERT_TRUE(memcmp(buf+p, "IHDR", 4)==0); p+=4;
    ASSERT_TRUE(p+ihdr_len+4 <= n);
    const unsigned char* ihdr = buf+p; p += ihdr_len;
    uint32_t ihdr_crc = be32(buf+p); p+=4;

    // Verify IHDR fields
    ASSERT_EQ_I(be32(ihdr+0), W);
    ASSERT_EQ_I(be32(ihdr+4), H);
    ASSERT_EQ_I(ihdr[8], 8);   // bit depth
    ASSERT_EQ_I(ihdr[9], 6);   // RGBA
    ASSERT_EQ_I(ihdr[10],0);
    ASSERT_EQ_I(ihdr[11],0);
    ASSERT_EQ_I(ihdr[12],0);

    // Verify IHDR CRC (type + data)
    unsigned char ihdr_type_data[4+13];
    memcpy(ihdr_type_data, "IHDR", 4);
    memcpy(ihdr_type_data+4, ihdr, 13);
    ASSERT_EQ_U32(crc32(ihdr_type_data, sizeof ihdr_type_data), ihdr_crc);

    // IDAT (may be single chunk per encoder)
    ASSERT_TRUE(p+8 <= n);
    uint32_t idat_len = be32(buf+p); p+=4;
    ASSERT_TRUE(memcmp(buf+p, "IDAT", 4)==0); p+=4;
    ASSERT_TRUE(p+idat_len+4 <= n);
    const unsigned char* idat = buf+p; p += idat_len;
    uint32_t idat_crc = be32(buf+p); p+=4;

    // Verify IDAT CRC
    unsigned char* tmp = (unsigned char*)malloc(4 + idat_len);
    memcpy(tmp, "IDAT", 4); memcpy(tmp+4, idat, idat_len);
    ASSERT_EQ_U32(crc32(tmp, 4+idat_len), idat_crc);
    free(tmp);

    // IDAT payload is a zlib stream with header 0x78 0x01 and stored blocks
    ASSERT_TRUE(idat_len >= 2+4);
    ASSERT_EQ_I(idat[0], 0x78);
    ASSERT_EQ_I(idat[1], 0x01);

    // The last 4 bytes of the zlib stream are Adler32 of raw scanlines
    uint32_t adl_file = be32(idat + idat_len - 4);

    // Rebuild raw scanlines to check Adler32: each row = [0x00][R][G][B][A] * W
    size_t row_bytes = 1 + W*4;
    size_t raw_size = H * row_bytes;
    unsigned char* raw = (unsigned char*)malloc(raw_size);
    for (uint32_t y=0; y<H; ++y) {
        unsigned char* row = raw + y*row_bytes;
        row[0] = 0x00;
        for (uint32_t x=0; x<W; ++x) {
            uint32_t pxx = px[y*W + x];
            row[1 + 4*x + 0] = (pxx>>24)&0xFF;
            row[1 + 4*x + 1] = (pxx>>16)&0xFF;
            row[1 + 4*x + 2] = (pxx>> 8)&0xFF;
            row[1 + 4*x + 3] = (pxx    )&0xFF;
        }
    }
    ASSERT_EQ_U32(adler32(raw, raw_size), adl_file);
    free(raw);

    // IEND
    ASSERT_TRUE(p+8 <= n);
    uint32_t iend_len = be32(buf+p); p+=4;
    ASSERT_EQ_I(iend_len, 0);
    ASSERT_TRUE(memcmp(buf+p, "IEND", 4)==0); p+=4;
    // CRC of "IEND" is fixed (0xAE426082)
    ASSERT_EQ_U32(be32(buf+p), 0xAE426082u); p+=4;

    // No trailing bytes
    ASSERT_EQ_I((long long)p, (long long)n);

    if (g_fail) {
        free(buf);
        fprintf(stderr, "FAILED (%d assertion%s)\n", g_fail, g_fail == 1 ? "" : "s");
        return 1;
    }
    puts("OK");

    free(buf);
}
