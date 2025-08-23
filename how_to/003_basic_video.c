#define CANVAS_IMPLEMENTATION
#include "../canvas.h"

#define WIDTH 1600
#define HEIGHT 900

static uint32_t pixels[WIDTH * HEIGHT];

int main() {
    const int FPS = 30;
    const int DURATION = 5;
    const int RADIUS = 50;
    const int total_frames = FPS * DURATION;

    Canvas c = create_canvas(WIDTH, HEIGHT, pixels);
    Y4MWriter *writer = y4m_start("out.y4m", c.width, c.height, FPS);

    for (int frame = 0; frame < total_frames; frame++) {
        clear_background(&c, 0x00222DFF);
        // 0 → 1
        float t = (float)frame / (total_frames - 1);

        clear_background(&c, 0x3222DFF);

        int x = (int)((RADIUS) + t * ((c.width - RADIUS) - RADIUS));
        int y = c.height / 2;

        canvas_circle_fill(&c, x, y, RADIUS, 0xFFFF00FF);

        y4m_write_frame(writer, &c);
    }

    y4m_end(writer);
    free_canvas(&c);
    return 0;
}
