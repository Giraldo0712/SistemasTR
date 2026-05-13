#ifndef LIBRARY_RGB_H
#define LIBRARY_RGB_H

void rgb_init(void);
void rgb_set_color(int r, int g, int b, int led);
void rgb_get_color(int led, int *r, int *g, int *b);

#endif