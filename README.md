# Canvas.h

A single-header C library for creating RGBA32 images and saving them as PNG files — no external dependencies.

## Features

* Single header, drop-in library (`canvas.h`)
* Supports 32-bit RGBA pixels (`0xRRGGBBAA`)
* Simple API for drawing and saving to PNG or YUV4MPEG2
* No dynamic allocation inside `create_canvas` - caller controls memory

## Quick Example

```c
#define CANVAS_IMPLEMENTATION
#include "canvas.h"

#define WIDTH 1600
#define HEIGHT 900

static uint32_t pixels[WIDTH * HEIGHT];

int main(void) {
    // Create canvas using caller-provided buffer
    Canvas c = create_canvas(WIDTH, HEIGHT, pixels);
    // Fill background
    clear_background(&c, 0x00222DFF);
    // Save PNG
    if (write_png_from_rgba32("out.png", c.pixels, c.width, c.height) != 0) {
        fprintf(stderr, "Failed to write PNG\n");
        return 1;
    }
    return 0;
}
```

---

### Mandelbrot

![mandelbret](demos/mandelbrot.png "Mandelbrot")

See [mandelbret](demos/mandelbrot.c)

## Memory Ownership

The `Canvas` struct does not allocate or free memory for `pixels`.

You must:

* Allocate the buffer yourself (malloc, static, or global array)
* Free it yourself if heap-allocated

⚠️ Note: Very large pixel arrays (e.g. 1600×900) may overflow the default stack. Use `malloc` or `static` storage for large images.
