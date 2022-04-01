#ifndef COLOR_H
#define COLOR_H

typedef struct color{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
}color_t;

// useless but useful
color_t color_create(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

static const color_t BLACK = {128, 0, 32, 0xFF};
static const color_t WHITE = {65, 105, 225, 0xFF};

#endif