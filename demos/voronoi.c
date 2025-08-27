#include <stdlib.h>
#include <time.h>

#define CANVASDEF static inline
#define CANVAS_IMPLEMENTATION
#include "../canvas.h"

#define WIDTH           1600
#define HEIGHT          900
#define SAMPLE_RADIUS   10
#define N_SAMPLES       4

typedef struct {
    size_t x, y;
    uint32_t color;
} Sample;

static uint32_t pixels[WIDTH * HEIGHT];
static Sample samples[N_SAMPLES] = {0};

void generate_random_samples() {
    for (size_t i = 0; i < N_SAMPLES; i++) {
        samples[i].x = SAMPLE_RADIUS + rand() % (WIDTH  - 2 * SAMPLE_RADIUS);
        samples[i].y = SAMPLE_RADIUS + rand() % (HEIGHT - 2 * SAMPLE_RADIUS);
        samples[i].color = RGB(rand() % 256, rand() % 256, rand() % 256);
    }
}

void color_by_nearest_sample(Canvas *c) {
    for (int y = 0; y < (int)c->height; ++y) {
        for (int x = 0; x < (int)c->width; ++x) {
            unsigned long dist = (unsigned long) -1;
            Sample sample;
            for (size_t i = 0; i < N_SAMPLES; ++i) {
                long dx = (long)x - (long)samples[i].x;
                long dy = (long)y - (long)samples[i].y;
                unsigned long d2 = (unsigned long)(dx * dx + dy * dy);
                if (d2 < dist) {
                    dist = d2;
                    sample = samples[i];
                }
            }
            canvas_putpixel(c, x, y, sample.color);
        }
    }
}

int main() {
    srand(time(NULL));

    Canvas c = create_canvas(WIDTH, HEIGHT, pixels);
    clear_background(&c, 0xFFFFFFFF);

    generate_random_samples();
    color_by_nearest_sample(&c);

    for (int i = 0; i < N_SAMPLES; ++i) {
        Sample sample = samples[i];
        canvas_circle_fill(&c, sample.x, sample.y, SAMPLE_RADIUS, 0x000000FF);
    }

    if (write_png_from_rgba32("vornoi.png", c.pixels, c.width, c.height) != 0) {
        fprintf(stderr, "Failed to write PNG\n");
        free_canvas(&c);
        return 1;
    }

    free_canvas(&c);
}
