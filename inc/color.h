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

static const color_t BLACK = {100, 149, 237, 0xFF};
static const color_t WHITE = {65, 105, 225, 0xFF};

static const color_t RED = {0xFF, 0, 0, 0xFF};
static const color_t GREEN = {0, 0xFF, 0, 0xFF};
static const color_t BLUE = {0, 0, 0xFF, 0xFF};

static const color_t TURN = {0xFF, 0xFF, 0xFF, 0xFF};

#endif