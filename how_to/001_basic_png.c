#define CANVAS_IMPLEMENTATION
#include "../canvas.h"

int main() {
    const size_t w = 1600;
    const size_t h = 900;
    Canvas c = create_canvas(w, h);
    clear_background(&c, 0x00222DFF);
    if (write_png_from_rgba32("test_out.png", c.pixels, c.width, c.height) != 0) {
        fprintf(stderr, "Failed to write PNG\n");
        free_canvas(&c);
        return 1;
    }
    free_canvas(&c);
    return 0;
}
