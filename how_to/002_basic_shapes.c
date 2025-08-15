#define CANVAS_IMPLEMENTATION
#include "../canvas.h"

#define WIDTH 1600
#define HEIGHT 900

static uint32_t pixels[WIDTH * HEIGHT];

int main() {
    Canvas c = create_canvas(WIDTH, HEIGHT, pixels);
    clear_background(&c, 0x00222DFF);

    Rectangle rec = {
        .x = 0,
        .y = 0,
        .w = 200,
        .h = 200
    };
    canvas_rect(&c, rec, 0xFF00FFFF);
    if (write_png_from_rgba32("test_out.png", c.pixels, c.width, c.height) != 0) {
        fprintf(stderr, "Failed to write PNG\n");
        free_canvas(&c);
        return 1;
    }
    free_canvas(&c);
    return 0;
}
