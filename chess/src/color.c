#include <color.h>

color_t color_create(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    color_t result = {r, g, b, a};
    return result;
}