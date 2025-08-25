#define CANVAS_IMPLEMENTATION
#include "../canvas.h"

#include <math.h>

#define WIDTH     1600
#define HEIGHT     900
#define MAX_ITERS 1000

static uint32_t pixels[WIDTH * HEIGHT];

static inline uint32_t palette(double t) {
    const double TAU = 6.2831853071795864769;
    double r = 0.5 + 0.5 * cos(TAU * (t + 0.00));
    double g = 0.5 + 0.5 * cos(TAU * (t + 0.33));
    double b = 0.5 + 0.5 * cos(TAU * (t + 0.67));
    if (r < 0) r = 0; 
    if (r > 1) r = 1;
    if (g < 0) g = 0; 
    if (g > 1) g = 1;
    if (b < 0) b = 0; 
    if (b > 1) b = 1;
    return RGB((uint8_t)(r * 255.0), (uint8_t)(g * 255.0), (uint8_t)(b * 255.0));
}

int main(void) {
    Canvas c = create_canvas(WIDTH, HEIGHT, pixels);

    const double xc = -0.75;
    const double yc =  0.00;
    const double xr =  1.75;
    const double yr =  xr * (double)HEIGHT / (double)WIDTH;

    for (int y = 0; y < HEIGHT; ++y) {
        double ci = yc + (((double)y / (HEIGHT - 1)) * 2.0 - 1.0) * yr;

        for (int x = 0; x < WIDTH; ++x) {
            double cr = xc + (((double)x / (WIDTH - 1)) * 2.0 - 1.0) * xr;

            double zr = 0.0, zi = 0.0;
            int n = 0;
            for (; n < MAX_ITERS; ++n) {
                double zr2 = zr * zr;
                double zi2 = zi * zi;
                if (zr2 + zi2 > 4.0) break;
                double two_zr_zi = 2.0 * zr * zi;
                zr = zr2 - zi2 + cr;
                zi = two_zr_zi + ci;
            }

            uint32_t color;
            if (n == MAX_ITERS) {
                color = RGBA(0, 0, 0, 255);
            } else {
                double zr2 = zr * zr, zi2 = zi * zi;
                double log_zn = 0.5 * log(zr2 + zi2);
                double nu = (double)n + 1.0 - log(log_zn) / log(2.0);
                double t = nu / (double)MAX_ITERS;
                color = palette(t);
            }
            canvas_putpixel(&c, x, y, color);
        }
    }

    if (write_png_from_rgba32("mandelbrot.png", c.pixels, c.width, c.height) != 0) {
        fprintf(stderr, "Failed to write PNG\n");
        free_canvas(&c);
        return 1;
    }

    free_canvas(&c);
    return 0;
}
