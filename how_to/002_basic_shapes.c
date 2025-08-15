#define CANVAS_IMPLEMENTATION
#include "../canvas.h"

#define WIDTH 1800
#define HEIGHT 900
#define BACKGROUND_COLOR 0xFBF8F8FF
#define SHAPE_COLORS 0x991914FF

static uint32_t pixels[WIDTH * HEIGHT];

int main() {
    Canvas c = create_canvas(WIDTH, HEIGHT, pixels);
    clear_background(&c, BACKGROUND_COLOR);
    canvas_rect_fill(&c, (Rectangle){
        .x = 10,
        .y = 10,
        .w = 580,
        .h = 430
    }, SHAPE_COLORS);
    canvas_rect(&c, (Rectangle){
        .x = 10,
        .y = 460,
        .w = 580,
        .h = 430
    }, SHAPE_COLORS);
    canvas_circle_fill(&c, 900, 225, 200, SHAPE_COLORS);
    canvas_circle(&c, 900, 675, 200, SHAPE_COLORS);
    canvas_triangle_fill(&c, 1200, 440, 1200, 10, 1790, 225, SHAPE_COLORS);
    canvas_triangle(&c, 1200, 890, 1200, 460, 1790, 675, SHAPE_COLORS);
    if (write_png_from_rgba32("test_out.png", c.pixels, c.width, c.height) != 0) {
        fprintf(stderr, "Failed to write PNG\n");
        free_canvas(&c);
        return 1;
    }
    free_canvas(&c);
    return 0;
}
