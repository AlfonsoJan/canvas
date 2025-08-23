#include <stdio.h>
#include <stdlib.h>

#define CANVASDEF static inline
#define CANVAS_IMPLEMENTATION
#include "../canvas.h"

#include "test.h"

int main(void) {
    uint32_t pix[H * W];
    Canvas c = create_canvas(W, H, pix);

    // clear
    clear_background(&c, RGBA(1, 2, 3, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, 0, 0, 0), RGBA(1, 2, 3, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, W - 1, H - 1, 0), RGBA(1, 2, 3, 255));

    // put/get pixel + bounds safety
    canvas_putpixel(&c, -1, 0, 0xDEADBEEF);
    canvas_putpixel(&c, 0, -1, 0xDEADBEEF);
    canvas_putpixel(&c, W, 0, 0xDEADBEEF);
    canvas_putpixel(&c, 0, H, 0xDEADBEEF);
    ASSERT_EQ_U32(canvas_getpixel(&c, 0, 0, 0), RGBA(1, 2, 3, 255));

    canvas_putpixel(&c, 2, 3, RGBA(9, 9, 9, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, 2, 3, 0), RGBA(9, 9, 9, 255));

    // hline clamps and draws inclusive
    clear_background(&c, 0);
    canvas_hline(&c, -5, W + 5, 4, RGBA(255, 0, 0, 255));
    for (int x = 0; x < W; x++) {
        ASSERT_EQ_U32(canvas_getpixel(&c, x, 4, 0), RGBA(255, 0, 0, 255));
    }
    ASSERT_EQ_U32(canvas_getpixel(&c, 0, 3, 0), 0);

    // vline clamps and draws inclusive
    clear_background(&c, 0);
    canvas_vline(&c, 7, -10, H + 10, RGBA(0, 255, 0, 255));
    for (int y = 0; y < H; y++) {
        ASSERT_EQ_U32(canvas_getpixel(&c, 7, y, 0), RGBA(0, 255, 0, 255));
    }
    ASSERT_EQ_U32(canvas_getpixel(&c, 6, 0, 0), 0);

    // line (Bresenham)
    clear_background(&c, 0);
    canvas_line(&c, 0, 0, 5, 0, RGBA(1, 0, 0, 255)); // horizontal
    canvas_line(&c, 0, 0, 0, 5, RGBA(1, 0, 0, 255)); // vertical
    canvas_line(&c, 0, 0, 5, 5, RGBA(1, 0, 0, 255)); // 45°
    canvas_line(&c, 5, 0, 0, 5, RGBA(1, 0, 0, 255)); // steep
    ASSERT_EQ_U32(canvas_getpixel(&c, 0, 0, 0), RGBA(1, 0, 0, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, 5, 0, 0), RGBA(1, 0, 0, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, 0, 5, 0), RGBA(1, 0, 0, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, 5, 5, 0), RGBA(1, 0, 0, 255));

    // rect (outline) + fill + clamping
    clear_background(&c, 0);
    Rectangle r = {2, 2, 5, 4};
    canvas_rect(&c, r, RGBA(0, 0, 255, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, 2, 2, 0), RGBA(0, 0, 255, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, 6, 5, 0), RGBA(0, 0, 255, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, 3, 3, 0), 0); // interior untouched by outline

    clear_background(&c, 0);
    Rectangle rf = {W - 3, H - 3, 10, 10};
    canvas_rect_fill(&c, rf, RGBA(8, 8, 8, 255));
    for (int y = H - 3; y < H; ++y) {
        for (int x = W - 3; x < W; ++x) {
            ASSERT_EQ_U32(canvas_getpixel(&c, x, y, 0), RGBA(8, 8, 8, 255));
        }
    }

    // circle (outline)
    clear_background(&c, 0);
    canvas_circle(&c, 8, 6, 3, RGBA(10, 10, 10, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, 11, 6, 0), RGBA(10, 10, 10, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, 5, 6, 0), RGBA(10, 10, 10, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, 8, 3, 0), RGBA(10, 10, 10, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, 8, 9, 0), RGBA(10, 10, 10, 255));

    // circle fill
    clear_background(&c, 0);
    canvas_circle_fill(&c, 8, 6, 3, RGBA(9, 9, 9, 255));
    for (int x = 5; x <= 11; ++x) {
        ASSERT_EQ_U32(canvas_getpixel(&c, x, 6, 0), RGBA(9, 9, 9, 255));
    }

    // triangle outline + fill variants (flat-top, flat-bottom, general)
    clear_background(&c, 0);
    canvas_triangle(&c, 2, 2, 10, 2, 6, 8, RGBA(77, 77, 77, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, 2, 2, 0), RGBA(77, 77, 77, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, 10, 2, 0), RGBA(77, 77, 77, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, 6, 8, 0), RGBA(77, 77, 77, 255));

    clear_background(&c, 0);
    // flat-bottom
    canvas_triangle_fill(&c, 4, 3, 8, 3, 6, 7, RGBA(123, 0, 0, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, 6, 5, 0), RGBA(123, 0, 0, 255));
    // flat-top
    clear_background(&c, 0);
    canvas_triangle_fill(&c, 6, 3, 4, 7, 8, 7, RGBA(0, 123, 0, 255));
    ASSERT_EQ_U32(canvas_getpixel(&c, 6, 6, 0), RGBA(0, 123, 0, 255));
    // general (split case)
    clear_background(&c, 0);
    canvas_triangle_fill(&c, 3, 2, 10, 6, 2, 9, RGBA(0, 0, 123, 255));
    ASSERT_TRUE(canvas_getpixel(&c, 5, 6, 0) != 0);

    // create/free canvas sanity
    free_canvas(&c);
    ASSERT_EQ_I(c.width, 0);
    ASSERT_EQ_I(c.height, 0);
    ASSERT_TRUE(c.pixels == NULL);


    if (g_fail) {
        fprintf(stderr, "FAILED (%d assertion%s)\n", g_fail, g_fail == 1 ? "" : "s");
        return 1;
    }
    puts("OK");
    return 0;
}
